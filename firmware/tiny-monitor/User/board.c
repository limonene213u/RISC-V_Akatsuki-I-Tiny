#include "board.h"

void board_init(void)
{
    GPIO_InitTypeDef gpio = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    gpio.GPIO_Pin = LED0_PIN | LED1_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &gpio);
    GPIO_ResetBits(LED_PORT, LED0_PIN | LED1_PIN);

    gpio.GPIO_Pin = BTN0_PIN | BTN1_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(BTN_PORT, &gpio);

    /* PB1/PB5, PB0, PC10/11, PC16/17 and PC18/19 are intentionally untouched. */
}

void board_led0_toggle(void) { LED_PORT->OUTDR ^= LED0_PIN; }
void board_led0_set(int on) { GPIO_WriteBit(LED_PORT, LED0_PIN, on ? Bit_SET : Bit_RESET); }
void board_led1_set(int on) { GPIO_WriteBit(LED_PORT, LED1_PIN, on ? Bit_SET : Bit_RESET); }

int board_button(unsigned int index)
{
    uint32_t pin = index == 0U ? BTN0_PIN : BTN1_PIN;
    return GPIO_ReadInputDataBit(BTN_PORT, pin) == Bit_RESET;
}
