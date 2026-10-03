#ifndef SRAM_23LC512_H
#define SRAM_23LC512_H
#include <stddef.h>
#include <stdint.h>
void sram_init(void);
void sram_read(uint16_t addr, uint8_t *data, size_t len);
void sram_write(uint16_t addr, const uint8_t *data, size_t len);
int sram_quick_test(void);
#endif
