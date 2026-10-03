#ifndef EEPROM_24FC512_H
#define EEPROM_24FC512_H
#include <stddef.h>
#include <stdint.h>
void eeprom_init(void);
int eeprom_detect(void);
int eeprom_read(uint16_t addr, uint8_t *data, size_t len);
int eeprom_write(uint16_t addr, const uint8_t *data, size_t len);
int eeprom_test_restore(uint16_t addr, size_t len);
#endif
