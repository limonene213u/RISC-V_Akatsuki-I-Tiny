#include "eeprom_24fc512.h"
#include "swi2c.h"
#include "debug.h"
#include <string.h>

#define EEPROM_ADDRESS 0x50U
#define EEPROM_PAGE_SIZE 128U
#define ACK_POLL_LIMIT 20U
#define TEST_MAX 32U

static int select_device(int read)
{
    return swi2c_start() && swi2c_write((uint8_t)((EEPROM_ADDRESS << 1) | (read ? 1U : 0U)));
}

int eeprom_detect(void) { int ok = select_device(0); swi2c_stop(); return ok; }
void eeprom_init(void) { swi2c_init(); }

int eeprom_read(uint16_t addr, uint8_t *data, size_t len)
{
    size_t i;
    if (len == 0U) return 1;
    if (!select_device(0) || !swi2c_write((uint8_t)(addr >> 8)) || !swi2c_write((uint8_t)addr)) goto fail;
    if (!select_device(1)) goto fail;
    for (i = 0; i < len; ++i) data[i] = swi2c_read(i + 1U < len);
    swi2c_stop(); return 1;
fail:
    swi2c_stop(); return 0;
}

static int wait_ready(void)
{
    unsigned int poll;
    for (poll = 0; poll < ACK_POLL_LIMIT; ++poll) {
        if (select_device(0)) { swi2c_stop(); return 1; }
        swi2c_stop(); Delay_Ms(1);
    }
    return 0;
}

int eeprom_write(uint16_t addr, const uint8_t *data, size_t len)
{
    while (len != 0U) {
        size_t room = EEPROM_PAGE_SIZE - ((size_t)addr % EEPROM_PAGE_SIZE);
        size_t chunk = len < room ? len : room, i;
        if (!select_device(0) || !swi2c_write((uint8_t)(addr >> 8)) || !swi2c_write((uint8_t)addr)) goto fail;
        for (i = 0; i < chunk; ++i) if (!swi2c_write(data[i])) goto fail;
        swi2c_stop();
        if (!wait_ready()) return 0;
        addr = (uint16_t)(addr + chunk); data += chunk; len -= chunk;
    }
    return 1;
fail:
    swi2c_stop(); return 0;
}

int eeprom_test_restore(uint16_t addr, size_t len)
{
    uint8_t backup[TEST_MAX], pattern[TEST_MAX], actual[TEST_MAX]; size_t i;
    if (len == 0U || len > TEST_MAX) return 0;
    if (!eeprom_read(addr, backup, len)) return 0;
    for (i = 0; i < len; ++i) pattern[i] = (uint8_t)(0xa5U ^ i);
    if (!eeprom_write(addr, pattern, len) || !eeprom_read(addr, actual, len) || memcmp(pattern, actual, len) != 0) {
        (void)eeprom_write(addr, backup, len); return 0;
    }
    if (!eeprom_write(addr, backup, len) || !eeprom_read(addr, actual, len)) return 0;
    return memcmp(backup, actual, len) == 0;
}
