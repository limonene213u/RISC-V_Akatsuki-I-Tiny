#include "swi2c.h"
#include "board.h"
#include "debug.h"

#define I2C_DELAY_US 3U
#define STRETCH_TIMEOUT 1000U

static void delay(void) { Delay_Us(I2C_DELAY_US); }
static void pin_drive_low(uint32_t pin)
{
    GPIO_InitTypeDef gpio = {0};
    GPIO_ResetBits(SWI2C_PORT, pin);
    gpio.GPIO_Pin = pin; gpio.GPIO_Mode = GPIO_Mode_Out_PP; gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SWI2C_PORT, &gpio);
}
static void pin_release(uint32_t pin)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.GPIO_Pin = pin; gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING; gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SWI2C_PORT, &gpio);
}
static void sda(int high) { if (high) pin_release(SWI2C_SDA_PIN); else pin_drive_low(SWI2C_SDA_PIN); }
static void scl_low(void) { pin_drive_low(SWI2C_SCL_PIN); }
static int scl_high(void)
{
    uint32_t timeout = STRETCH_TIMEOUT;
    pin_release(SWI2C_SCL_PIN);
    while (GPIO_ReadInputDataBit(SWI2C_PORT, SWI2C_SCL_PIN) == Bit_RESET) {
        if (timeout == 0U) return 0;
        --timeout;
    }
    delay();
    return 1;
}

void swi2c_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = SWI2C_SDA_PIN | SWI2C_SCL_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SWI2C_PORT, &gpio);
    GPIO_SetBits(SWI2C_PORT, SWI2C_SDA_PIN | SWI2C_SCL_PIN);
}

int swi2c_start(void)
{
    sda(1); if (!scl_high()) return 0; sda(0); delay(); scl_low(); return 1;
}
void swi2c_stop(void) { sda(0); delay(); if (scl_high()) { sda(1); delay(); } }

int swi2c_write(uint8_t value)
{
    unsigned int bit;
    for (bit = 0; bit < 8U; ++bit) {
        sda((value & 0x80U) != 0U); delay();
        if (!scl_high()) return 0;
        scl_low(); value <<= 1;
    }
    sda(1); delay(); if (!scl_high()) return 0;
    bit = GPIO_ReadInputDataBit(SWI2C_PORT, SWI2C_SDA_PIN);
    scl_low();
    return bit == 0U;
}

uint8_t swi2c_read(int ack)
{
    unsigned int bit; uint8_t value = 0;
    sda(1);
    for (bit = 0; bit < 8U; ++bit) {
        value <<= 1; delay();
        if (!scl_high()) return 0xffU;
        if (GPIO_ReadInputDataBit(SWI2C_PORT, SWI2C_SDA_PIN) != Bit_RESET) value |= 1U;
        scl_low();
    }
    sda(!ack); delay(); (void)scl_high(); scl_low(); sda(1);
    return value;
}
