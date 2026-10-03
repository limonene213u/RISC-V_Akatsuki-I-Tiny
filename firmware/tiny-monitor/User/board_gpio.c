#include "board_gpio.h"
#include "ch32x035.h"
#include <string.h>

typedef struct { const board_pin_t *pin; gpio_safe_mode_t mode; } user_state_t;
static user_state_t states[7];
static size_t state_count;

static GPIO_TypeDef *port_for(char port) { return port == 'A' ? GPIOA : port == 'B' ? GPIOB : GPIOC; }
static uint32_t mask_for(uint8_t number) { return UINT32_C(1) << number; }
static user_state_t *state_for(const board_pin_t *pin)
{
    size_t i; for (i = 0; i < state_count; ++i) if (states[i].pin == pin) return &states[i]; return 0;
}
void board_gpio_init(void)
{
    const board_pin_t *table; size_t count, i;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    table = pin_policy_table(&count); state_count = 0;
    for (i = 0; i < count; ++i) if (pin_policy_is_user(&table[i])) {
        states[state_count].pin = &table[i]; states[state_count].mode = GPIO_SAFE_INPUT; ++state_count;
        board_gpio_reset(&table[i]);
    }
}
int board_gpio_set_mode(const board_pin_t *pin, gpio_safe_mode_t mode, int initial_level)
{
    GPIO_InitTypeDef gpio = {0}; user_state_t *state;
    if (!pin_policy_is_user(pin) || (mode == GPIO_SAFE_PULLDOWN && !pin->supports_pulldown)) return 0;
    state = state_for(pin); if (!state) return 0;
    gpio.GPIO_Pin = mask_for(pin->number); gpio.GPIO_Speed = GPIO_Speed_50MHz;
    if (mode == GPIO_SAFE_OUTPUT) {
        GPIO_WriteBit(port_for(pin->port), gpio.GPIO_Pin, initial_level ? Bit_SET : Bit_RESET);
        gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    } else if (mode == GPIO_SAFE_PULLUP) gpio.GPIO_Mode = GPIO_Mode_IPU;
    else if (mode == GPIO_SAFE_PULLDOWN) gpio.GPIO_Mode = GPIO_Mode_IPD;
    else gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(port_for(pin->port), &gpio); state->mode = mode; return 1;
}
int board_gpio_write(const board_pin_t *pin, int level)
{
    user_state_t *state = state_for(pin);
    if (!state || state->mode != GPIO_SAFE_OUTPUT) return 0;
    GPIO_WriteBit(port_for(pin->port), mask_for(pin->number), level ? Bit_SET : Bit_RESET); return 1;
}
int board_gpio_read(const board_pin_t *pin)
{
    return GPIO_ReadInputDataBit(port_for(pin->port), mask_for(pin->number)) != Bit_RESET;
}
gpio_safe_mode_t board_gpio_mode(const board_pin_t *pin)
{
    user_state_t *state = state_for(pin); return state ? state->mode : GPIO_SAFE_INPUT;
}
const char *board_gpio_mode_name(gpio_safe_mode_t mode)
{
    static const char *const names[] = {"input", "pullup", "pulldown", "output"}; return names[(unsigned)mode];
}
void board_gpio_reset(const board_pin_t *pin) { (void)board_gpio_set_mode(pin, GPIO_SAFE_INPUT, 0); }
void board_gpio_reset_all(void)
{
    size_t i; for (i = 0; i < state_count; ++i) board_gpio_reset(states[i].pin);
}
