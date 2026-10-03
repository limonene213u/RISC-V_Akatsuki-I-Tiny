#include "memory_commands.h"
#include "memory_policy.h"
#include "parser.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern uint8_t __user_scratch_start, __user_scratch_end;
extern uint8_t __exec_buffer_start, __exec_buffer_end;
extern uint8_t _data_vma, _ebss, _heap_end, _susrstack, _eusrstack;

static int width_from_command(const char *name, const char *base, uint32_t *width)
{
    size_t n = strlen(base);
    if (strncmp(name, base, n) != 0) return 0;
    if (name[n] == '\0' || !strcmp(name + n, ".w")) *width = 4U;
    else if (!strcmp(name + n, ".h")) *width = 2U;
    else if (!strcmp(name + n, ".b")) *width = 1U;
    else return 0;
    return 1;
}

static uint32_t read_value(uint32_t address, uint32_t width)
{
    if (width == 1U) return *(volatile const uint8_t *)(uintptr_t)address;
    if (width == 2U) return *(volatile const uint16_t *)(uintptr_t)address;
    return *(volatile const uint32_t *)(uintptr_t)address;
}

static void write_value(uint32_t address, uint32_t width, uint32_t value)
{
    if (width == 1U) *(volatile uint8_t *)(uintptr_t)address = (uint8_t)value;
    else if (width == 2U) *(volatile uint16_t *)(uintptr_t)address = (uint16_t)value;
    else *(volatile uint32_t *)(uintptr_t)address = value;
}

static int parse_address_count(char **v, size_t c, uint32_t *address, uint32_t *count)
{
    *count = 1U;
    return parser_parse_u32(v[1], address) && (c == 2U || parser_parse_u32(v[2], count));
}

static void deny(const char *reason) { printf("error: %s\r\n", reason); }

static void cmd_map(const char *part)
{
    const memory_region_t *regions; size_t count, i;
    if (!part || !strcmp(part, "physical")) {
        puts("CH32X035G8U6 Memory Map (end address is inclusive)");
        regions = memory_physical_regions(&count);
        for (i = 0; i < count; ++i) printf("%-12s %08lx-%08lx  %-10s\r\n", regions[i].name,
            (unsigned long)regions[i].start, (unsigned long)(regions[i].end - 1U), regions[i].access);
        puts("BOOT ALIAS maps Flash or system memory according to boot selection.");
        puts("Use 'map ram', 'map mmio', or 'map policy' for details.");
    } else if (!strcmp(part, "flash")) {
        puts("BOOT ALIAS   00000000-0000f7ff  R-  boot dependent");
        puts("CODE FLASH   08000000-0800f7ff  R-  62 KiB user area");
        puts("SYSTEM       1fff0000-1fff0cff  --  protected");
        puts("VENDOR       1ffff700-1ffff7ff  --  protected");
        puts("OPTION       1ffff800-1ffff8ff  --  protected");
    } else if (!strcmp(part, "ram")) {
        printf("USER SCRATCH  %08lx-%08lx  %lu bytes  RW-\r\n", (unsigned long)(uintptr_t)&__user_scratch_start,
            (unsigned long)((uintptr_t)&__user_scratch_end - 1U), (unsigned long)(&__user_scratch_end - &__user_scratch_start));
        printf("EXEC BUFFER   %08lx-%08lx  %lu bytes  R-X\r\n", (unsigned long)(uintptr_t)&__exec_buffer_start,
            (unsigned long)((uintptr_t)&__exec_buffer_end - 1U), (unsigned long)(&__exec_buffer_end - &__exec_buffer_start));
        printf("MONITOR DATA  %08lx-%08lx              protected\r\n", (unsigned long)(uintptr_t)&_data_vma,
            (unsigned long)((uintptr_t)&_ebss - 1U));
        printf("MONITOR FREE  %08lx-%08lx              protected\r\n", (unsigned long)(uintptr_t)&_ebss,
            (unsigned long)((uintptr_t)&_heap_end - 1U));
        printf("MONITOR STACK %08lx-%08lx  %lu bytes  protected\r\n", (unsigned long)(uintptr_t)&_susrstack,
            (unsigned long)((uintptr_t)&_eusrstack - 1U), (unsigned long)(&_eusrstack - &_susrstack));
        puts("PIOC SRAM sharing: disabled");
    } else if (!strcmp(part, "mmio")) {
        regions = memory_mmio_regions(&count);
        puts("BASE      PERIPHERAL    READ POLICY");
        for (i = 0; i < count; ++i) printf("%08lx  %-13s %s\r\n", (unsigned long)regions[i].start, regions[i].name, regions[i].access);
        puts("Listed does not mean readable; only the register allowlist is accessible.");
    } else if (!strcmp(part, "policy")) {
        puts("md/cmp/crc32: boot alias, Code Flash, scratch, exec; allow-listed MMIO");
        puts("mw/cp destination: USER SCRATCH only");
        puts("EXEC writes: reserved for future word/asm commands");
        puts("System/Vendor/Option/Monitor/Core: protected");
        puts("MMIO writes: protected; PIOC: disabled");
    } else puts("usage: map [ram|flash|mmio|policy]");
}

static void cmd_md(uint32_t width, uint32_t address, uint32_t count)
{
    const char *reason = 0; uint32_t i, per_line = width == 1U ? 16U : 4U;
    if (!memory_read_allowed(address, count, width, &reason)) { deny(reason); return; }
    for (i = 0; i < count; ++i) {
        if ((i % per_line) == 0U) printf("%08lx:", (unsigned long)(address + i * width));
        printf(width == 1U ? " %02lx" : width == 2U ? " %04lx" : " %08lx", (unsigned long)read_value(address + i * width, width));
        if ((i % per_line) == per_line - 1U || i + 1U == count) puts("");
    }
}

static void cmd_mw(uint32_t width, uint32_t address, uint32_t value, uint32_t count)
{
    const char *reason = 0; uint32_t i, limit = width == 1U ? UINT32_C(0xff) : width == 2U ? UINT32_C(0xffff) : UINT32_MAX;
    if (value > limit) { puts("error: value does not fit access width"); return; }
    if (!memory_write_allowed(address, count, width, &reason)) { deny(reason); return; }
    for (i = 0; i < count; ++i) write_value(address + i * width, width, value);
    printf("OK: %lu element(s) written\r\n", (unsigned long)count);
}

static void cmd_cp(uint32_t width, uint32_t src, uint32_t dst, uint32_t count)
{
    const char *reason = 0; uint32_t bytes, i;
    if (!memory_range_valid(src, count, width, &bytes)) { puts("error: source range overflows"); return; }
    if (!memory_read_allowed(src, count, width, &reason)) { deny(reason); return; }
    if (!memory_write_allowed(dst, count, width, &reason)) { deny(reason); return; }
    if (dst > src && dst < src + bytes) for (i = count; i != 0U; --i) write_value(dst + (i - 1U) * width, width, read_value(src + (i - 1U) * width, width));
    else for (i = 0; i < count; ++i) write_value(dst + i * width, width, read_value(src + i * width, width));
    printf("copied: %lu bytes\r\n", (unsigned long)bytes);
}

static void cmd_cmp(uint32_t width, uint32_t a, uint32_t b, uint32_t count)
{
    const char *reason = 0; uint32_t bytes, i, av, bv;
    if (!memory_range_valid(a, count, width, &bytes) || !memory_read_allowed(a, count, width, &reason) || !memory_read_allowed(b, count, width, &reason)) { deny(reason ? reason : "range overflows"); return; }
    for (i = 0; i < count; ++i) { av = read_value(a + i * width, width); bv = read_value(b + i * width, width); if (av != bv) {
        printf("different at offset 0x%lx\r\n0x%08lx = 0x%08lx\r\n0x%08lx = 0x%08lx\r\n", (unsigned long)(i * width),
            (unsigned long)(a + i * width), (unsigned long)av, (unsigned long)(b + i * width), (unsigned long)bv); return; } }
    printf("equal: %lu bytes\r\n", (unsigned long)bytes);
}

static void cmd_crc32(uint32_t address, uint32_t length)
{
    const char *reason = 0; uint32_t crc = UINT32_MAX, i, bit;
    if (!memory_read_allowed(address, length, 1U, &reason)) { deny(reason); return; }
    for (i = 0; i < length; ++i) { crc ^= read_value(address + i, 1U); for (bit = 0; bit < 8U; ++bit) crc = (crc >> 1) ^ ((crc & 1U) ? UINT32_C(0xedb88320) : 0U); }
    printf("CRC32 = 0x%08lx\r\n", (unsigned long)~crc);
}

int memory_commands_execute(char **v, size_t c)
{
    uint32_t width, a, b, n = 1U;
    if (!strcmp(v[0], "map")) { if (c <= 2U) cmd_map(c == 2U ? v[1] : 0); else puts("usage: map [ram|flash|mmio|policy]"); return 1; }
    if (width_from_command(v[0], "md", &width)) { if ((c == 2U || c == 3U) && parse_address_count(v, c, &a, &n)) cmd_md(width, a, n); else puts("usage: md.b|h|w <address> [count]"); return 1; }
    if (width_from_command(v[0], "mw", &width)) { if ((c == 3U || c == 4U) && parser_parse_u32(v[1], &a) && parser_parse_u32(v[2], &b) && (c == 3U || parser_parse_u32(v[3], &n))) cmd_mw(width, a, b, n); else puts("usage: mw.b|h|w <address> <value> [count]"); return 1; }
    if (width_from_command(v[0], "cp", &width)) { if (c == 4U && parser_parse_u32(v[1], &a) && parser_parse_u32(v[2], &b) && parser_parse_u32(v[3], &n)) cmd_cp(width, a, b, n); else puts("usage: cp.b|h|w <source> <destination> <count>"); return 1; }
    if (width_from_command(v[0], "cmp", &width)) { if (c == 4U && parser_parse_u32(v[1], &a) && parser_parse_u32(v[2], &b) && parser_parse_u32(v[3], &n)) cmd_cmp(width, a, b, n); else puts("usage: cmp.b|h|w <address1> <address2> <count>"); return 1; }
    if (!strcmp(v[0], "crc32")) { if (c == 3U && parser_parse_u32(v[1], &a) && parser_parse_u32(v[2], &n)) cmd_crc32(a, n); else puts("usage: crc32 <address> <length>"); return 1; }
    return 0;
}
