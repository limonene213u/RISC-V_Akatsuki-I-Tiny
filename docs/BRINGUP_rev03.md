# Akatsuki I Tiny Rev0.3 — Bring-up checklist

## 1. Unpowered checks
- [ ] +5V to GND is not shorted
- [ ] +3V3 to GND is not shorted
- [ ] C2 / C8 polarity correct
- [ ] D1-D4 polarity correct
- [ ] U2 orientation/pinout correct
- [ ] U3/U4/U7 orientation correct
- [ ] JP1 open
- [ ] no external output connected to J7-5

## 2. Power
- [ ] SW4 MAIN PWR OFF
- [ ] connect USB-C power
- [ ] switch MAIN PWR ON
- [ ] verify +5V rail
- [ ] verify +3V3 rail

## 3. WCH-Link
- [ ] J4-2 GND
- [ ] J4-3 DIO → PC18
- [ ] J4-4 DCK → PC19
- [ ] target detected
- [ ] erase/program/verify/reset succeeds

## 4. UART
- [ ] PB10 TX → adapter RX
- [ ] PB11 RX ← adapter TX
- [ ] 115200 8N1, no flow control, local echo off
- [ ] RISC-V Monitor banner and `rv>` prompt appear

## 5. GPIO
- [ ] LED0/LED1
- [ ] BTN0/BTN1
- [ ] protected/debug/console pins are not accidentally repurposed

## 6. Buses/peripherals
- [ ] SPI SRAM
- [ ] software I2C / EEPROM / display
- [ ] SD interface
- [ ] USB data
- [ ] USB HOST power only after MCU boot

## Project tracking

Linear: https://linear.app/aets-magi/project/akatsuki-i-tiny-083b4a1547ce
