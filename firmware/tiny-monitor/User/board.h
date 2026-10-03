#ifndef AKATSUKI_BOARD_H
#define AKATSUKI_BOARD_H

#include "ch32x035.h"

#define BOARD_NAME          "Akatsuki I Tiny"
#define BOARD_REVISION      "Rev0.3"

#define LED_PORT            GPIOA
#define LED0_PIN            GPIO_Pin_0
#define LED1_PIN            GPIO_Pin_1
#define BTN_PORT            GPIOA
#define BTN0_PIN            GPIO_Pin_2
#define BTN1_PIN            GPIO_Pin_3

#define SRAM_PORT           GPIOA
#define SRAM_CS_PIN         GPIO_Pin_4
#define SRAM_SCK_PIN        GPIO_Pin_5
#define SRAM_MISO_PIN       GPIO_Pin_6
#define SRAM_MOSI_PIN       GPIO_Pin_7

#define SWI2C_PORT          GPIOB
#define SWI2C_SDA_PIN       GPIO_Pin_6
#define SWI2C_SCL_PIN       GPIO_Pin_7

#define UART_PORT           GPIOB
#define UART_TX_PIN         GPIO_Pin_10
#define UART_RX_PIN         GPIO_Pin_11

/* Reserved persistent/scratch areas at the top of each 64 KiB device. */
#define EEPROM_BOOT_ADDR    UINT32_C(0xfff0)
#define SRAM_TEST_ADDR      UINT32_C(0xfff0)

void board_init(void);
void board_led0_toggle(void);
void board_led0_set(int on);
void board_led1_set(int on);
int board_button(unsigned int index);

#endif
