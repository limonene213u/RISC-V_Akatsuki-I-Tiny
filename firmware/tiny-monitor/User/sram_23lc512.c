#include "sram_23lc512.h"
#include "board.h"
#include "spi.h"
#include <string.h>

#define SRAM_READ  0x03U
#define SRAM_WRITE 0x02U
#define SRAM_WRMR  0x01U
#define SRAM_SEQ   0x40U

static void address(uint8_t command, uint16_t addr)
{
    spi_select();
    (void)spi_transfer(command);
    (void)spi_transfer((uint8_t)(addr >> 8));
    (void)spi_transfer((uint8_t)addr);
}

void sram_init(void)
{
    spi_init();
    spi_select();
    (void)spi_transfer(SRAM_WRMR);
    (void)spi_transfer(SRAM_SEQ);
    spi_deselect();
}

void sram_read(uint16_t addr, uint8_t *data, size_t len)
{
    address(SRAM_READ, addr);
    while (len-- != 0U) *data++ = spi_transfer(0xffU);
    spi_deselect();
}

void sram_write(uint16_t addr, const uint8_t *data, size_t len)
{
    address(SRAM_WRITE, addr);
    while (len-- != 0U) (void)spi_transfer(*data++);
    spi_deselect();
}

int sram_quick_test(void)
{
    static const uint8_t pattern[] = {0x00, 0xff, 0x55, 0xaa, 0x3c, 0xc3, 0x69, 0x96};
    uint8_t backup[sizeof(pattern)], actual[sizeof(pattern)];
    sram_read((uint16_t)SRAM_TEST_ADDR, backup, sizeof(backup));
    sram_write((uint16_t)SRAM_TEST_ADDR, pattern, sizeof(pattern));
    sram_read((uint16_t)SRAM_TEST_ADDR, actual, sizeof(actual));
    sram_write((uint16_t)SRAM_TEST_ADDR, backup, sizeof(backup));
    return memcmp(pattern, actual, sizeof(pattern)) == 0;
}
