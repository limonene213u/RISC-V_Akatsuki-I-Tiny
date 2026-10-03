#include "console.h"
#include "board.h"
#include <stdio.h>

void console_putc(char ch)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {}
    USART_SendData(USART1, (uint8_t)ch);
}

void console_puts(const char *text)
{
    while (*text != '\0') console_putc(*text++);
}

void console_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    USART_InitTypeDef uart = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOB, ENABLE);
    gpio.GPIO_Pin = UART_TX_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(UART_PORT, &gpio);
    gpio.GPIO_Pin = UART_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART_PORT, &gpio);

    uart.USART_BaudRate = 115200;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    uart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

int console_poll_line(char *line, size_t capacity)
{
    static size_t used;
    static int previous_was_cr;
    uint8_t ch;

    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET) return 0;
    ch = (uint8_t)USART_ReceiveData(USART1);
    if (ch == '\n' && previous_was_cr) {
        previous_was_cr = 0;
        return 0;
    }
    if (ch == '\r' || ch == '\n') {
        previous_was_cr = (ch == '\r');
        line[used] = '\0';
        used = 0;
        console_putc('\r');
        console_putc('\n');
        return 1;
    }
    previous_was_cr = 0;
    if ((ch == '\b' || ch == 0x7fU) && used != 0U) {
        --used;
        console_putc('\b');
        console_putc(' ');
        console_putc('\b');
    } else if (ch >= 0x20U && ch < 0x7fU && used + 1U < capacity) {
        line[used++] = (char)ch;
        console_putc((char)ch);
    } else if (ch >= 0x20U && ch < 0x7fU) {
        console_putc('\a');
    }
    return 0;
}
