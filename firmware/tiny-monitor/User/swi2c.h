#ifndef SWI2C_H
#define SWI2C_H
#include <stdint.h>
void swi2c_init(void);
int swi2c_start(void);
void swi2c_stop(void);
int swi2c_write(uint8_t value);
uint8_t swi2c_read(int ack);
#endif
