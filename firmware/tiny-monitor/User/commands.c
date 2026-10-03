#include "commands.h"
#include "board.h"
#include "board_gpio.h"
#include "console.h"
#include "eeprom_24fc512.h"
#include "parser.h"
#include "pin_policy.h"
#include "sram_23lc512.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LINE_SIZE 128U
#define MAX_ARGS 35U
#define IO_CHUNK 32U
#define BOOT_MAGIC UINT32_C(0x54494b41)
#define MEMO_ADDR UINT32_C(0xfd00)
#define MEMO_SIZE 128U
typedef struct { uint32_t magic, count, check; } boot_record_t;
static int sram_ok, eeprom_present;
static int heartbeat_enabled = 1;

static uint32_t boot_check(uint32_t n) { return BOOT_MAGIC ^ n ^ UINT32_C(0xa55a3cc3); }
static void help(void)
{
    console_puts(
        "help                              show this text\r\n"
        "info                              show board and device status\r\n"
        "sr <address> [length]             dump SRAM (default 16 bytes)\r\n"
        "sw <address> <byte> [byte...]     write SRAM\r\n"
        "er <address> [length]             dump EEPROM (default 16 bytes)\r\n"
        "ew <address> <byte> [byte...]     write EEPROM\r\n"
        "save <ee_addr> <sram_addr> <len>  copy SRAM to EEPROM\r\n"
        "load <sram_addr> <ee_addr> <len>  copy EEPROM to SRAM\r\n"
        "memo <text>                       save text at EEPROM 0xfd00\r\n"
        "st                                test SRAM and restore data\r\n"
        "et [address [length]]             test EEPROM and restore data\r\n"
        "pins                              show board pin policy\r\n"
        "gpio list                         list USER GPIO pins\r\n"
        "gpio status|read <pin>            inspect a pin\r\n"
        "gpio mode <pin> input|pullup      set safe input mode\r\n"
        "gpio mode <pin> output <0|1>      set glitch-safe output\r\n"
        "gpio write <pin> <0|1>            write an output-mode USER pin\r\n"
        "gpio reset <pin|all>              return USER GPIO to input\r\n"
        "led 0 on|off|toggle|auto          control LED0/heartbeat\r\n"
        "button <0|1>                      read a board button\r\n"
        "\r\n"
        "Numbers: decimal by default, hexadecimal with 0x prefix.\r\n"
        "Memory range: 0x0000..0xffff; et length: 1..0x20 bytes.\r\n"
        "GPIO is allow-listed by board policy; raw register writes are unavailable.\r\n");
}
static int range_args(char **v, size_t argc, uint32_t *a, uint32_t *n)
{
    *n = 16U;
    if (!parser_parse_u32(v[1], a) || (argc == 3U && !parser_parse_u32(v[2], n))) {
        puts("error: invalid number"); return 0;
    }
    if (*n == 0U || !parser_range_valid(*a, *n)) { puts("error: range must be inside 0x0000..0xffff"); return 0; }
    return 1;
}
static int eeprom_write_range_valid(uint32_t a, uint32_t n)
{
    return parser_range_valid(a, n) && (a + n <= EEPROM_BOOT_ADDR || a >= EEPROM_BOOT_ADDR + sizeof(boot_record_t));
}
static void dump(int ee, uint32_t a, uint32_t n)
{
    uint8_t data[IO_CHUNK]; uint32_t chunk, i;
    while (n) {
        chunk = n < IO_CHUNK ? n : IO_CHUNK;
        if (ee) { if (!eeprom_read((uint16_t)a, data, chunk)) { puts("error: EEPROM I/O failed"); return; } }
        else sram_read((uint16_t)a, data, chunk);
        printf("%04lx:", (unsigned long)a);
        for (i = 0; i < chunk; ++i) printf(" %02x", data[i]);
        puts(""); a += chunk; n -= chunk;
    }
}
static void write_bytes(int ee, uint32_t a, char **v, size_t n)
{
    uint8_t data[MAX_ARGS - 2U]; size_t i;
    if (!parser_range_valid(a, (uint32_t)n) || (ee && !eeprom_write_range_valid(a, (uint32_t)n))) { puts("error: range must be inside 0x0000..0xffff"); return; }
    for (i = 0; i < n; ++i) { uint32_t x;
        if (!parser_parse_u32(v[i + 2U], &x)) { puts("error: invalid byte"); return; }
        if (x > 0xffU) { puts("error: byte must be 0..0xff"); return; } data[i] = (uint8_t)x;
    }
    if (ee && !eeprom_write((uint16_t)a, data, n)) { puts("error: EEPROM I/O failed"); return; }
    if (!ee) sram_write((uint16_t)a, data, n);
    puts("OK");
}
static void copy_memory(int to_ee, uint32_t dst, uint32_t src, uint32_t n)
{
    uint8_t data[IO_CHUNK]; uint32_t chunk;
    while (n) { chunk = n < IO_CHUNK ? n : IO_CHUNK;
        if (to_ee) { sram_read((uint16_t)src, data, chunk); if (!eeprom_write((uint16_t)dst, data, chunk)) goto fail; }
        else { if (!eeprom_read((uint16_t)src, data, chunk)) goto fail; sram_write((uint16_t)dst, data, chunk); }
        dst += chunk; src += chunk; n -= chunk;
    } puts("OK"); return;
fail: puts("error: memory I/O failed");
}
static void boot_counter(void)
{
    boot_record_t r, verify;
    if (!eeprom_present) { puts("Boot count: unavailable (EEPROM not detected)"); return; }
    if (!eeprom_read((uint16_t)EEPROM_BOOT_ADDR, (uint8_t *)&r, sizeof(r))) { puts("Boot count: unavailable (EEPROM read failed)"); return; }
    if (r.magic != BOOT_MAGIC || r.check != boot_check(r.count)) r.count = 0;
    if (r.count == UINT32_MAX) { puts("Boot count: unavailable (counter overflow)"); return; }
    r.magic = BOOT_MAGIC; ++r.count; r.check = boot_check(r.count);
    if (!eeprom_write((uint16_t)EEPROM_BOOT_ADDR, (uint8_t *)&r, sizeof(r)) ||
        !eeprom_read((uint16_t)EEPROM_BOOT_ADDR, (uint8_t *)&verify, sizeof(verify)) || memcmp(&r, &verify, sizeof(r))) {
        puts("Boot count: unavailable (EEPROM write failed)"); return;
    }
    printf("Boot count: %lu\r\n", (unsigned long)r.count);
}
static void print_pin_error(const board_pin_t *pin)
{
    if (pin->policy == PIN_LOCKED) printf("error: %s is locked (%s)\r\n", pin->name, pin->reason);
    else if (pin->policy == PIN_INPUT_ONLY) printf("error: %s is input-only (%s)\r\n", pin->name, pin->function);
    else printf("error: %s is reserved for %s\r\n", pin->name, pin->function);
}
static const board_pin_t *require_pin(const char *name)
{
    const board_pin_t *pin = pin_policy_find(name);
    if (!pin) puts("error: unknown pin");
    return pin;
}
static const board_pin_t *require_user_pin(const char *name)
{
    const board_pin_t *pin = require_pin(name);
    if (pin && !pin_policy_is_user(pin)) { print_pin_error(pin); return 0; }
    return pin;
}
static void cmd_pins(void)
{
    const board_pin_t *table; size_t count, i;
    console_puts("PIN   FUNCTION       POLICY      REASON\r\n");
    table = pin_policy_table(&count);
    for (i = 0; i < count; ++i)
        printf("%-5s %-14s %-11s %s\r\n", table[i].name, table[i].function,
               pin_policy_name(table[i].policy), table[i].reason);
}
static void cmd_gpio(char **v, size_t c)
{
    const board_pin_t *pin; uint32_t level;
    if (c == 2U && !strcmp(v[1], "list")) {
        const board_pin_t *table; size_t count, i; table = pin_policy_table(&count);
        for (i = 0; i < count; ++i) if (pin_policy_is_user(&table[i])) printf("%s\r\n", table[i].name);
        return;
    }
    if (c == 3U && !strcmp(v[1], "status")) {
        pin = require_pin(v[2]); if (!pin) return;
        printf("%s\r\nfunction: %s\r\npolicy: %s\r\n", pin->name, pin->function, pin_policy_name(pin->policy));
        if (pin_policy_is_user(pin)) printf("mode: %s\r\nlevel: %u\r\n", board_gpio_mode_name(board_gpio_mode(pin)), board_gpio_read(pin));
        else printf("reason: %s\r\n", pin->reason);
        return;
    }
    if (c == 3U && !strcmp(v[1], "read")) {
        pin = require_user_pin(v[2]); if (pin) printf("%s = %u\r\n", pin->name, board_gpio_read(pin)); return;
    }
    if (c == 4U && !strcmp(v[1], "write")) {
        pin = require_user_pin(v[2]); if (!pin) return;
        if (!parser_parse_u32(v[3], &level) || level > 1U) { puts("usage: gpio write <pin> <0|1>"); return; }
        if (!board_gpio_write(pin, (int)level)) { printf("error: %s is not configured as output\r\n", pin->name); return; }
        printf("%s = %lu\r\n", pin->name, (unsigned long)level); return;
    }
    if (c >= 4U && !strcmp(v[1], "mode")) {
        gpio_safe_mode_t mode;
        pin = require_user_pin(v[2]); if (!pin) return;
        if (!strcmp(v[3], "output")) {
            if (c != 5U || !parser_parse_u32(v[4], &level) || level > 1U) { puts("usage: gpio mode <pin> output <0|1>"); return; }
            mode = GPIO_SAFE_OUTPUT;
        } else {
            if (c != 4U) { puts("usage: gpio mode <pin> input|pullup|pulldown"); return; }
            level = 0;
            if (!strcmp(v[3], "input")) mode = GPIO_SAFE_INPUT;
            else if (!strcmp(v[3], "pullup")) mode = GPIO_SAFE_PULLUP;
            else if (!strcmp(v[3], "pulldown")) mode = GPIO_SAFE_PULLDOWN;
            else { puts("error: invalid GPIO mode"); return; }
        }
        if (mode == GPIO_SAFE_PULLDOWN && !pin->supports_pulldown) {
            printf("error: %s does not support internal pull-down on CH32X035\r\n", pin->name); return;
        }
        if (!board_gpio_set_mode(pin, mode, (int)level)) { puts("error: GPIO mode change rejected by board policy"); return; }
        printf("%s mode = %s", pin->name, board_gpio_mode_name(mode));
        if (mode == GPIO_SAFE_OUTPUT) printf(", level = %lu", (unsigned long)level);
        puts(""); return;
    }
    if (c == 3U && !strcmp(v[1], "reset")) {
        if (!strcmp(v[2], "all")) { board_gpio_reset_all(); puts("USER GPIO reset to input"); return; }
        pin = require_user_pin(v[2]); if (pin) { board_gpio_reset(pin); printf("%s reset to input\r\n", pin->name); } return;
    }
    puts("usage: gpio list|status <pin>|read <pin>|mode <pin> ...|write <pin> <0|1>|reset <pin|all>");
}
static void cmd_led(char **v, size_t c)
{
    uint32_t index;
    if (c != 3U || !parser_parse_u32(v[1], &index) || index > 1U) { puts("usage: led <0|1> on|off|toggle|auto"); return; }
    if (index == 1U) { puts("error: LED1 is reserved for system status"); return; }
    if (!strcmp(v[2], "auto")) { heartbeat_enabled = 1; puts("LED0 heartbeat enabled"); return; }
    heartbeat_enabled = 0;
    if (!strcmp(v[2], "on")) board_led0_set(1);
    else if (!strcmp(v[2], "off")) board_led0_set(0);
    else if (!strcmp(v[2], "toggle")) board_led0_toggle();
    else { puts("usage: led 0 on|off|toggle|auto"); heartbeat_enabled = 1; return; }
    puts("LED0 manual mode (use 'led 0 auto' to restore heartbeat)");
}
static void cmd_button(char **v, size_t c)
{
    uint32_t index;
    if (c != 2U || !parser_parse_u32(v[1], &index) || index > 1U) { puts("usage: button <0|1>"); return; }
    printf("BTN%lu = %s\r\n", (unsigned long)index, board_button((unsigned)index) ? "pressed" : "released");
}
void commands_execute(char *line)
{
    char *v[MAX_ARGS]; size_t c = parser_tokenize(line, v, MAX_ARGS); uint32_t a, b, n;
    if (!c) return;
    if (c > MAX_ARGS) { puts("error: too many arguments"); return; }
    { char *p = v[0]; while (*p != '\0') { *p = (char)tolower((unsigned char)*p); ++p; } }
    if (!strcmp(v[0], "help") && c == 1U) help();
    else if (!strcmp(v[0], "info") && c == 1U)
        printf("%s %s\r\nSRAM: %s\r\nEEPROM: %s\r\nBTN0: %s BTN1: %s\r\n", BOARD_NAME, BOARD_REVISION,
            sram_ok ? "OK" : "FAIL", eeprom_present ? "DETECTED" : "NOT FOUND",
            board_button(0) ? "pressed" : "released", board_button(1) ? "pressed" : "released");
    else if ((!strcmp(v[0], "sr") || !strcmp(v[0], "er")) && (c == 2U || c == 3U)) { if (range_args(v, c, &a, &n)) dump(v[0][0] == 'e', a, n); }
    else if ((!strcmp(v[0], "sw") || !strcmp(v[0], "ew")) && c >= 3U) {
        if (!parser_parse_u32(v[1], &a)) puts("error: invalid address"); else write_bytes(v[0][0] == 'e', a, v, c - 2U);
    } else if ((!strcmp(v[0], "save") || !strcmp(v[0], "load")) && c == 4U) {
        if (!parser_parse_u32(v[1], &a) || !parser_parse_u32(v[2], &b) || !parser_parse_u32(v[3], &n)) puts("error: invalid number");
        else if (!parser_range_valid(a, n) || !parser_range_valid(b, n) || (v[0][0] == 's' && !eeprom_write_range_valid(a, n))) puts("error: range must be inside 0x0000..0xffff");
        else copy_memory(v[0][0] == 's', a, b, n);
    } else if (!strcmp(v[0], "memo") && c >= 2U) {
        uint8_t memo[MEMO_SIZE] = {0}; size_t i, used = 0;
        for (i = 1; i < c; ++i) { size_t len = strlen(v[i]);
            if (used != 0U && used + 1U < sizeof(memo)) memo[used++] = ' ';
            if (len > sizeof(memo) - 1U - used) len = sizeof(memo) - 1U - used;
            memcpy(memo + used, v[i], len); used += len;
        }
        if (!eeprom_write((uint16_t)MEMO_ADDR, memo, sizeof(memo))) puts("error: EEPROM I/O failed"); else puts("memo saved at 0xfd00");
    }
    else if (!strcmp(v[0], "st") && c == 1U) { sram_ok = sram_quick_test(); puts(sram_ok ? "SRAM test OK (restored)" : "SRAM test FAIL"); }
    else if (!strcmp(v[0], "et") && c <= 3U) { a = UINT32_C(0xfce0); n = 16U;
        if ((c >= 2U && !parser_parse_u32(v[1], &a)) || (c == 3U && !parser_parse_u32(v[2], &n))) puts("error: invalid number");
        else if (!n || n > 32U || !eeprom_write_range_valid(a, n)) puts("error: invalid EEPROM test range");
        else puts(eeprom_test_restore((uint16_t)a, n) ? "EEPROM test OK (restored and verified)" : "error: EEPROM test/restore failed");
    } else if (!strcmp(v[0], "pins") && c == 1U) cmd_pins();
    else if (!strcmp(v[0], "gpio")) cmd_gpio(v, c);
    else if (!strcmp(v[0], "led")) cmd_led(v, c);
    else if (!strcmp(v[0], "button")) cmd_button(v, c);
    else puts("error: unknown command or arguments (type 'help')");
}
void commands_init(void)
{
    board_gpio_init(); sram_init(); eeprom_init();
    sram_ok = sram_quick_test(); eeprom_present = eeprom_detect(); board_led1_set(sram_ok && eeprom_present);
    printf("SRAM: %s\r\nEEPROM: %s\r\n",
        sram_ok ? "OK" : "FAIL", eeprom_present ? "DETECTED" : "NOT FOUND");
    boot_counter();
}
int commands_heartbeat_enabled(void) { return heartbeat_enabled; }
