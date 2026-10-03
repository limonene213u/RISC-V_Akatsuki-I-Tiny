#ifndef AKATSUKI_SPI_H
#define AKATSUKI_SPI_H
#include <stdint.h>
void spi_init(void);
void spi_select(void);
void spi_deselect(void);
uint8_t spi_transfer(uint8_t value);
#endif
