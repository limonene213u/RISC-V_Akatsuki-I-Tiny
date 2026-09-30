# Akatsuki I Tiny Rev0.3 — Pinout

| U1 pin | MCU pin | Rev0.3 function |
| ---: | --- | --- |
| 1 | PC15 | NC |
| 2 | VDD | +3V3 |
| 3 | PC0 | EXP_PC0 |
| 4 | PC3 | EXP_PC3 |
| 5 | PA0 | LED0 |
| 6 | PA1 | LED1 |
| 7 | PA2 | BTN0 |
| 8 | PA3 | BTN1 |
| 9 | PA4 | SRAM_CS |
| 10 | PA5 | SPI_SCK |
| 11 | PA6 | SPI_MISO |
| 12 | PA7 | SPI_MOSI |
| 13 | PB0 | EXP_PB0 |
| 14 | PB3 | SD_CS |
| 15 | PB4 | EXP_PB4 |
| 16 | PB1/PB5 | NC; internally shorted, never drive as outputs |
| 17 | PB6 | I2C_SDA (software I2C) |
| 18 | PB7 | I2C_SCL (software I2C) |
| 19 | PB8 | EXP_PB8 |
| 20 | PB9 | EXP_PB9 |
| 21 | PB10 | UART_TX |
| 22 | PB11 | UART_RX |
| 23 | PB12 | EXP_PB12 |
| 24 | PC19 | DBG_DCK / SWCLK |
| 25 | PC18 | DBG_DIO / SWDIO |
| 26 | PC16/UDM | USB_DM; PC11 must remain floating input |
| 27 | PC17/UDP | USB_DP; PC10 must remain floating input |
| 28 | PC14 | EXP_PC14 |

## Expansion J7

| Pin | Net |
| ---: | --- |
| 1 | +3V3 |
| 2 | GND |
| 3 | EXP_PC0 |
| 4 | EXP_PC3 |
| 5 | EXP_PB0 |
| 6 | EXP_PB4 |
| 7 | EXP_PB8 |
| 8 | EXP_PB9 |
| 9 | EXP_PB12 |
| 10 | EXP_PC14 |

## J7-5 warning

Rev0.3 routes PA7 as SPI_MOSI while also exposing PB0 at J7-5. Because of the CH32X035G8U6 package/internal bonding relationship, J7-5 must not be treated as an independently driven GPIO while SPI_MOSI is in use.

Until the mitigation is finalized:
- do not externally drive J7-5;
- do not configure PB0 as an output in firmware;
- consider protecting PB0 in the RISC-V Monitor GPIO policy.

Tracked in Linear: https://linear.app/aets-magi/issue/MASTER-57/j7-5-exp-pb0をspi-mosi競合から保護
