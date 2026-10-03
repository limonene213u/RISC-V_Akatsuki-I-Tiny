#include "memory_policy.h"

static const memory_region_t physical[] = {
    { UINT32_C(0x00000000), UINT32_C(0x0000f800), "BOOT ALIAS", "R-" },
    { UINT32_C(0x08000000), UINT32_C(0x0800f800), "CODE FLASH", "R-" },
    { UINT32_C(0x1fff0000), UINT32_C(0x1fff0d00), "SYSTEM", "--" },
    { UINT32_C(0x1ffff700), UINT32_C(0x1ffff800), "VENDOR", "--" },
    { UINT32_C(0x1ffff800), UINT32_C(0x1ffff900), "OPTION", "--" },
    { UINT32_C(0x20000000), UINT32_C(0x20005000), "SRAM", "policy" },
    { UINT32_C(0x40000000), UINT32_C(0x60000000), "MMIO", "restricted" },
    { UINT32_C(0xe0000000), UINT32_C(0xe0100000), "CORE", "--" }
};

static const memory_region_t mmio[] = {
    { UINT32_C(0x40000000), UINT32_C(0x40000400), "TIM2", "restricted" },
    { UINT32_C(0x40000400), UINT32_C(0x40000800), "TIM3", "restricted" },
    { UINT32_C(0x40002c00), UINT32_C(0x40003000), "WWDG", "restricted" },
    { UINT32_C(0x40003000), UINT32_C(0x40003400), "IWDG", "restricted" },
    { UINT32_C(0x40004400), UINT32_C(0x40004800), "USART2", "restricted" },
    { UINT32_C(0x40004800), UINT32_C(0x40004c00), "USART3", "restricted" },
    { UINT32_C(0x40004c00), UINT32_C(0x40005000), "USART4", "restricted" },
    { UINT32_C(0x40005400), UINT32_C(0x40005800), "I2C1", "restricted" },
    { UINT32_C(0x40007000), UINT32_C(0x40007400), "PWR", "restricted" },
    { UINT32_C(0x40010000), UINT32_C(0x40010400), "AFIO", "restricted" },
    { UINT32_C(0x40010400), UINT32_C(0x40010800), "EXTI", "restricted" },
    { UINT32_C(0x40010800), UINT32_C(0x40010c00), "GPIOA", "safe registers only" },
    { UINT32_C(0x40010c00), UINT32_C(0x40011000), "GPIOB", "safe registers only" },
    { UINT32_C(0x40011000), UINT32_C(0x40011400), "GPIOC", "safe registers only" },
    { UINT32_C(0x40012400), UINT32_C(0x40012800), "ADC/TouchKey", "restricted" },
    { UINT32_C(0x40012c00), UINT32_C(0x40013000), "TIM1", "restricted" },
    { UINT32_C(0x40013000), UINT32_C(0x40013400), "SPI", "restricted" },
    { UINT32_C(0x40013800), UINT32_C(0x40013c00), "USART1", "restricted" },
    { UINT32_C(0x40020000), UINT32_C(0x40020400), "DMA", "restricted" },
    { UINT32_C(0x40021000), UINT32_C(0x40021400), "RCC", "restricted" },
    { UINT32_C(0x40022000), UINT32_C(0x40022400), "FLASH IF", "restricted" },
    { UINT32_C(0x40023400), UINT32_C(0x40023800), "USBFS", "restricted" },
    { UINT32_C(0x40026000), UINT32_C(0x40026400), "OPA", "restricted" },
    { UINT32_C(0x40026400), UINT32_C(0x40026800), "AWU", "restricted" },
    { UINT32_C(0x40026c00), UINT32_C(0x40027000), "PIOC", "disabled" },
    { UINT32_C(0x40027000), UINT32_C(0x40027400), "USBPD", "restricted" }
};

static int inside(uint32_t address, uint32_t bytes, uint32_t start, uint32_t end)
{
    return address >= start && address < end && bytes <= end - address;
}

int memory_range_valid(uint32_t address, uint32_t count, uint32_t width, uint32_t *bytes)
{
    uint32_t total;
    if (count == 0U || (width != 1U && width != 2U && width != 4U) || count > UINT32_MAX / width) return 0;
    total = count * width;
    if (address > UINT32_MAX - (total - 1U)) return 0;
    if (bytes) *bytes = total;
    return 1;
}

static int aligned(uint32_t address, uint32_t width)
{
    return (address & (width - 1U)) == 0U;
}

static int safe_gpio_read(uint32_t address, uint32_t count, uint32_t width)
{
    uint32_t base, offset;
    if (count != 1U || width != 4U || address < UINT32_C(0x40010800) || address >= UINT32_C(0x40011400)) return 0;
    base = address & UINT32_C(0xfffffc00);
    if (base != UINT32_C(0x40010800) && base != UINT32_C(0x40010c00) && base != UINT32_C(0x40011000)) return 0;
    offset = address - base;
    return offset == 0U || offset == 4U || offset == 8U || offset == 12U;
}

int memory_read_allowed(uint32_t address, uint32_t count, uint32_t width, const char **reason)
{
    uint32_t bytes;
    if (!memory_range_valid(address, count, width, &bytes)) { *reason = "address range overflows"; return 0; }
    if (!aligned(address, width)) { *reason = width == 2U ? "address must be 2-byte aligned" : "address must be 4-byte aligned"; return 0; }
    if (inside(address, bytes, UINT32_C(0x00000000), UINT32_C(0x0000f800)) ||
        inside(address, bytes, UINT32_C(0x08000000), UINT32_C(0x0800f800)) ||
        inside(address, bytes, UINT32_C(0x20000000), UINT32_C(0x20001400))) return 1;
    if (safe_gpio_read(address, count, width)) return 1;
    if (inside(address, bytes, UINT32_C(0x20001400), UINT32_C(0x20005000))) *reason = "monitor memory is protected";
    else if (inside(address, bytes, UINT32_C(0x40000000), UINT32_C(0x60000000))) *reason = "MMIO register is not on the safe-read allowlist";
    else *reason = "region is protected or unmapped";
    return 0;
}

int memory_write_allowed(uint32_t address, uint32_t count, uint32_t width, const char **reason)
{
    uint32_t bytes;
    if (!memory_range_valid(address, count, width, &bytes)) { *reason = "address range overflows"; return 0; }
    if (!aligned(address, width)) { *reason = width == 2U ? "address must be 2-byte aligned" : "address must be 4-byte aligned"; return 0; }
    if (inside(address, bytes, UINT32_C(0x20000000), UINT32_C(0x20001000))) return 1;
    if (inside(address, bytes, UINT32_C(0x20001000), UINT32_C(0x20001400))) *reason = "execution buffer is protected; use 'word' or 'asm'";
    else if (inside(address, bytes, UINT32_C(0x20001400), UINT32_C(0x20005000))) *reason = "monitor memory is protected";
    else if (inside(address, bytes, UINT32_C(0x40000000), UINT32_C(0x60000000))) *reason = "MMIO writes are protected";
    else *reason = "region is read-only, protected, or unmapped";
    return 0;
}

const memory_region_t *memory_physical_regions(size_t *count)
{
    *count = sizeof(physical) / sizeof(physical[0]);
    return physical;
}

const memory_region_t *memory_mmio_regions(size_t *count)
{
    *count = sizeof(mmio) / sizeof(mmio[0]);
    return mmio;
}
