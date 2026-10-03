# Akatsuki I Tiny

Akatsuki I Tiny is a small RISC-V learning and experimentation board based on the **WCH CH32X035G8U6 (QingKe V4C / RV32IMAC)**.

This repository is the hardware source and manufacturing archive for Akatsuki I Tiny as a product, not for one PCB revision only. New board revisions should be added without deleting the historical files for older revisions.

## Current hardware

**Current board: Rev0.3 THT Edition**

Rev0.3 is designed around hand assembly and replaceable through-hole / module parts where practical.

Main functions:

- CH32X035G8U6 CPU module
- WCH-Link programming/debug header
- UART console
- GPIO LEDs and buttons
- software I2C display / EEPROM bus
- SPI SRAM
- SPI/SD expansion
- USB device data lines and USB host connector
- 2x5 expansion header
- manual MAIN PWR and HOST PWR control

## Rev0.3 quick pin map

| Function | CH32X035G8U6 |
| --- | --- |
| LED0 / LED1 | PA0 / PA1 |
| BTN0 / BTN1 | PA2 / PA3 |
| SRAM_CS | PA4 |
| SPI_SCK / MISO / MOSI | PA5 / PA6 / PA7 |
| SD_CS | PB3 |
| I2C SDA / SCL | PB6 / PB7 (software I2C) |
| UART TX / RX | PB10 / PB11 |
| WCH-Link DCK / DIO | PC19 / PC18 |
| USB DM / DP | PC16 / PC17 |

### Important Rev0.3 note

J7-5 is labeled **EXP_PB0**. On CH32X035G8U6, the PA7/PB0 package bonding relationship means this pin must not be treated as an independent externally-driven GPIO while PA7 is used as SPI_MOSI.

For Rev0.3 bring-up, **do not externally drive J7-5**. The hardware/firmware mitigation is tracked in Linear.

## Repository layout

```
hardware/
  rev03/
    kicad/            KiCad project metadata / library tables
    manufacturing/    manufacturing archive destination
    source/            full KiCad source archive destination
    netlist_rev03.csv
    parts_list_rev03.csv
docs/
  BOM_rev03.md
  PINOUT_rev03.md
  BRINGUP_rev03.md
firmware/
  tiny-monitor/       CH32X035G8U6 UART monitor and host-side tests
```

## Firmware

The `firmware/tiny-monitor/` directory contains the Akatsuki I Tiny monitor for
MounRiver Studio. It provides a UART console, board-aware GPIO policy, SPI SRAM
and I2C EEPROM commands, and host-side tests for hardware-independent logic.

See [`firmware/tiny-monitor/README.md`](firmware/tiny-monitor/README.md) for the
terminal settings, build procedure, command reference, and safety policy.

## Project management

Linear project: https://linear.app/aets-magi/project/akatsuki-i-tiny-083b4a1547ce

Assembly/BOM document: https://linear.app/aets-magi/document/akatsuki-i-tiny-実装表bom-8386743a65c7

Current tracked work includes Rev0.3 pin validation, J7-5 protection policy, first assembly/bring-up, and integration with the CH32X035 RISC-V Monitor.

## Bring-up order

1. Check +3.3V/GND and +5V/GND for shorts.
2. Validate the power section.
3. Bring up WCH-Link on PC19/PC18.
4. Bring up UART on PB10/PB11.
5. Boot the CH32X035 RISC-V Monitor.
6. Test LEDs/buttons/GPIO.
7. Test SPI SRAM.
8. Test software I2C.
9. Test USB / SD last.

## Binary design archives

The text BOM/netlist/docs are committed. The two binary ZIP archives still need to be uploaded to the repository because the current connector cannot reliably transfer the original binary bytes.

Expected files:

- `hardware/rev03/manufacturing/gerber_rev03_JLC.zip`
  - size: 161,207 bytes
  - SHA-256: `fbb4d96957f28013e82998b69a4798d8ce7ebaeb7e506b686b80bedc80d15ccd`
- `hardware/rev03/source/akatsuki_tiny_rev03.zip`
  - size: 7,618,559 bytes
  - SHA-256: `3f05fe586889200f3471e59771c409332d25a38a4daf7a1f5751d7d55efa80fc`

Do not substitute an older Rev0.2 archive.

## Status

Rev0.3 boards exist physically and are currently being assembled and brought up.
