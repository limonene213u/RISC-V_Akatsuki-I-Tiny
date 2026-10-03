#include "pin_policy.h"
#include <ctype.h>
#include <string.h>

static const board_pin_t pins[] = {
    {"PA0", 'A', 0, "LED0", PIN_BOARD_FUNCTION, "heartbeat/manual LED", 1},
    {"PA1", 'A', 1, "LED1", PIN_BOARD_FUNCTION, "system status LED", 1},
    {"PA2", 'A', 2, "BTN0", PIN_INPUT_ONLY, "button to GND", 1},
    {"PA3", 'A', 3, "BTN1", PIN_INPUT_ONLY, "button to GND", 1},
    {"PA4", 'A', 4, "SRAM_CS", PIN_BOARD_FUNCTION, "23LC512 chip select", 1},
    {"PA5", 'A', 5, "SPI_SCK", PIN_BOARD_FUNCTION, "shared SPI bus", 1},
    {"PA6", 'A', 6, "SPI_MISO", PIN_BOARD_FUNCTION, "shared SPI bus", 1},
    {"PA7", 'A', 7, "SPI_MOSI", PIN_BOARD_FUNCTION, "shared SPI bus", 1},
    {"PB0", 'B', 0, "EXP_PB0", PIN_LOCKED, "shared with PA7/SPI_MOSI", 0},
    {"PB1", 'B', 1, "internal", PIN_LOCKED, "internally shorted with PB5", 0},
    {"PB3", 'B', 3, "SD_CS", PIN_BOARD_FUNCTION, "SD MISO gate control", 0},
    {"PB4", 'B', 4, "EXP_PB4", PIN_USER, "J7 user GPIO", 0},
    {"PB5", 'B', 5, "internal", PIN_LOCKED, "internally shorted with PB1", 0},
    {"PB6", 'B', 6, "I2C_SDA", PIN_BOARD_FUNCTION, "EEPROM software I2C", 0},
    {"PB7", 'B', 7, "I2C_SCL", PIN_BOARD_FUNCTION, "EEPROM software I2C", 0},
    {"PB8", 'B', 8, "EXP_PB8", PIN_USER, "J7 user GPIO", 0},
    {"PB9", 'B', 9, "EXP_PB9", PIN_USER, "J7 user GPIO", 0},
    {"PB10", 'B', 10, "UART_TX", PIN_BOARD_FUNCTION, "monitor console", 0},
    {"PB11", 'B', 11, "UART_RX", PIN_BOARD_FUNCTION, "monitor console", 0},
    {"PB12", 'B', 12, "EXP_PB12", PIN_USER, "J7 user GPIO", 0},
    {"PC0", 'C', 0, "EXP_PC0", PIN_USER, "J7 user GPIO", 0},
    {"PC3", 'C', 3, "EXP_PC3", PIN_USER, "J7 user GPIO", 0},
    {"PC10", 'C', 10, "shared", PIN_LOCKED, "internally shared with PC17", 0},
    {"PC11", 'C', 11, "shared", PIN_LOCKED, "internally shared with PC16", 0},
    {"PC14", 'C', 14, "EXP_PC14", PIN_USER, "J7 user GPIO", 0},
    {"PC16", 'C', 16, "USB_DM", PIN_LOCKED, "USB and shared with PC11", 1},
    {"PC17", 'C', 17, "USB_DP/BOOT", PIN_LOCKED, "USB/BOOT and shared with PC10", 1},
    {"PC18", 'C', 18, "DBG_DIO", PIN_LOCKED, "debugger/programming interface", 0},
    {"PC19", 'C', 19, "DBG_DCK", PIN_LOCKED, "debugger/programming interface", 0}
};

static int same_name(const char *a, const char *b)
{
    while (*a && *b) { if (toupper((unsigned char)*a++) != toupper((unsigned char)*b++)) return 0; }
    return *a == '\0' && *b == '\0';
}
const board_pin_t *pin_policy_find(const char *name)
{
    size_t i; if (!name) return 0;
    for (i = 0; i < sizeof(pins) / sizeof(pins[0]); ++i) if (same_name(name, pins[i].name)) return &pins[i];
    return 0;
}
const board_pin_t *pin_policy_table(size_t *count)
{
    if (count) *count = sizeof(pins) / sizeof(pins[0]);
    return pins;
}
const char *pin_policy_name(pin_policy_t policy)
{
    static const char *const names[] = {"USER", "INPUT_ONLY", "BOARD", "LOCKED"};
    return names[(unsigned)policy];
}
int pin_policy_is_user(const board_pin_t *pin) { return pin && pin->policy == PIN_USER; }
int pin_policy_mode_request_valid(const board_pin_t *pin, const char *mode,
                                  int has_initial_value, uint32_t initial_value)
{
    if (!pin_policy_is_user(pin) || !mode) return 0;
    if (!strcmp(mode, "output")) return has_initial_value && initial_value <= 1U;
    if (has_initial_value) return 0;
    if (!strcmp(mode, "input") || !strcmp(mode, "pullup")) return 1;
    if (!strcmp(mode, "pulldown")) return pin->supports_pulldown != 0U;
    return 0;
}
