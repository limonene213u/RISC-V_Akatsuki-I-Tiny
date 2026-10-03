#ifndef TINY_BOARD_GPIO_H
#define TINY_BOARD_GPIO_H
#include "pin_policy.h"

typedef enum { GPIO_SAFE_INPUT, GPIO_SAFE_PULLUP, GPIO_SAFE_PULLDOWN, GPIO_SAFE_OUTPUT } gpio_safe_mode_t;
void board_gpio_init(void);
int board_gpio_set_mode(const board_pin_t *pin, gpio_safe_mode_t mode, int initial_level);
int board_gpio_write(const board_pin_t *pin, int level);
int board_gpio_read(const board_pin_t *pin);
gpio_safe_mode_t board_gpio_mode(const board_pin_t *pin);
const char *board_gpio_mode_name(gpio_safe_mode_t mode);
void board_gpio_reset(const board_pin_t *pin);
void board_gpio_reset_all(void);

#endif
