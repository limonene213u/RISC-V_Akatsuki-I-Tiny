# Akatsuki I Tiny Rev0.3 — Assembly BOM

Source of truth for quantities/part names: `hardware/rev03/parts_list_rev03.csv`.

## IC / module

| Ref | Qty | Part | Notes |
| --- | ---: | --- | --- |
| U1 | 1 | AE-QFN28-DIP, CH32X035G8U6 | Akizuki 131018; 1x15 sockets x2 recommended |
| U2 | 1 | TA48033S | 3.3V LDO, TO-220 |
| U3 | 1 | 24FC512-I/P | I2C EEPROM, DIP8 |
| U4 | 1 | 23LC512-I/P | SPI SRAM, DIP8 |
| U7 | 1 | 74HC125 | DIP14, SD MISO gate |
| U5 | 0 | USBLC6-2SC6 | Optional/DNP; signal path remains connected |

## Passive parts

- 5.1kΩ x2: R1,R2
- 1kΩ x3: R3,R4,R7
- 4.7kΩ x2: R5,R6
- 10kΩ x5: R8,R9,R10,R11,R14
- 2.2kΩ x1: R15
- 4.7µF ceramic x2: C1,C6
- 47µF 16V electrolytic x1: C2
- 0.1µF ceramic x4: C3,C4,C5,C10
- 10µF ceramic x1: C9
- 100µF 16V electrolytic x1: C8
- 0.5A PPTC x1: F1

## User-interface / connectors

- 3mm LED x4: D1-D4
- 6x6mm tact switch x2: SW1,SW2
- SPDT slide switch x2: SW3,SW4
- USB Type-C DIP kit x1: J1, Akizuki 115426
- 1x4 header x1: J2 USB HOST
- 1x5 header x1: J3 DISPLAY
- 1x4 header x1: J4 WCH-LINK
- 1x3 header x1: J5 UART
- 1x6 header x1: J6 SPI/SD
- 2x5 header x1: J7 EXP
- 1x2 header + jumper x1: JP1

## Suggested assembly order

1. Resistors
2. Ceramic capacitors
3. IC sockets
4. U2 / F1
5. LEDs / tact switches / slide switches
6. Electrolytic capacitors
7. USB-C module / headers
8. U1 socket and CH32X035 module
9. U3 / U4 / U7 into sockets

## Before first power

- Check +3V3 ↔ GND and +5V ↔ GND for shorts.
- Confirm C2/C8 polarity.
- Confirm LED polarity.
- Confirm TA48033S pinout and orientation.
- Leave JP1 open for normal USB-C powered operation.
- Do not externally drive J7-5 until the EXP_PB0 / PA7 policy is resolved.
