# Akatsuki I Tiny Monitor v0.1

Akatsuki I Tiny Rev0.3をUART端末からbring-upする、CH32X035G8U6向けbare-metal firmwareです。WCH EVT / MounRiver Studio系のstartup、linker、StdPeriphを使用し、Arduino、RTOS、`ch32fun`には依存しません。

現在のv0.1は、安全なGPIO操作、23LC512 SPI SRAM、24FC512 I2C EEPROMを中心としたhardware monitorです。将来は機械語、RISC-V assembly、register、native memory、MMIOを実CPU上で学ぶ「RISC-V Machine Monitor」へ発展させます。

## Design philosophy

このMonitorの目標は、低レベル操作を無制限に許可することではありません。学習に必要な範囲を見える形で公開しながら、Monitor自身や基板を壊し得る操作をpolicy layerで拒否します。

```text
Machine code
    ↓
RISC-V assembly
    ↓
Registers / load-store
    ↓
Native memory / MMIO
    ↓
GPIO and actual hardware
```

GPIO commandは主役ではなく、Lチカやdigital sensor確認のための初心者向けショートカットです。将来は、同じ動作をregister表示、MMIO観察、assembly実行へ段階的に掘り下げられる構成を目指します。

参考設計は [CH32X035 RISC-V Monitor](https://github.com/limonene213u/risc-v_CH32X035G8U6) です。console/parser/commandsの分離、`usage:`／`error:`形式、安全なscratch RAM、U-mode/PMP、trap recoveryの考え方を継承します。assemblerやCPU実行機能は現在のv0.1にはまだ統合されていません。

## Hardware source of truth

- `hardware/rev03/netlist_rev03.csv`
- `hardware/rev03/README_rev03.md`
- `docs/PINOUT_rev03.md`

firmwareのpin定義より上記Rev0.3資料を優先します。矛盾が見つかった場合は推測で変更せず、`HARDWARE REVIEW REQUIRED`として扱います。

## Build

MounRiver Studio 2付属のRISC-V Embedded GCCとGNU Makeを使用します。

```powershell
& 'C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin\make.exe'
```

別環境では`TOOLCHAIN_DIR`または`PREFIX`を指定します。成果物は次の通りです。

```text
build/tiny-monitor.elf
build/tiny-monitor.hex
build/tiny-monitor.bin
build/tiny-monitor.map
```

MounRiver Studioで外部からファイルが追加・変更された場合は、プロジェクトをRefreshしてから`Project → Clean...`、続けてBuildしてください。`tests/`はfirmwareのリンク対象外です。

host GCCが利用できる環境では次を実行します。

```powershell
make test
```

parser、64 KiB境界、CR/LF/CRLF、pin policyを検証します。

## Flash and wiring

WCH-LinkEをJ4へ接続します。

| J4 | WCH-LinkE |
|---|---|
| J4-2 GND | GND |
| J4-3 DBG_DIO | DIO |
| J4-4 DBG_DCK | DCK |

WCH-LinkEから給電する場合だけJP1を接続します。別電源とWCH-LinkEの同時給電は避け、GNDは共通にしてください。MounRiver StudioまたはWCH-LinkUtilityでELF/HEXを書き込みます。

UARTは3.3 V TTLです。5 V UART信号を接続しないでください。

| J5 | USB-UART adapter |
|---|---|
| J5-1 GND | GND |
| J5-2 UART_TX | RX |
| J5-3 UART_RX | TX |

## Terminal UX

- 115200 baud
- 8 data bits、no parity、1 stop bit
- flow controlなし
- local echo off
- firmware側echo
- CR、LF、CRLF対応
- Backspace、Delete対応
- ANSI escape sequence不要
- 最大入力長127文字
- prompt: `tiny> `

数値はbare表記が10進、`0x` prefix付きが16進です。command名は大文字・小文字を区別しません。

```text
tiny> info
tiny> sr 0x1000 16
tiny> sw 0x1000 0xaa 0x55
```

## Boot status

起動時にはboard revision、SRAM test、EEPROM detection、boot counterを表示します。

```text
Akatsuki I Tiny Monitor
Rev0.3
SRAM: OK
EEPROM: DETECTED
Boot count: 42
Type 'help' for commands.

tiny>
```

`EEPROM: DETECTED`は0x50からACKを受信したことだけを示します。EEPROM R/W確認済みという意味ではありません。LED0はheartbeat、LED1は`SRAM quick R/W OK + EEPROM detected`です。

## Current command set

### System and board

```text
help
info
pins
led 0 on|off|toggle|auto
button <0|1>
```

LED1はsystem status専用なので手動操作できません。LED0を手動操作するとheartbeatが停止し、`led 0 auto`で再開します。

### External SPI SRAM

```text
sr <address> [length]
sw <address> <byte> [byte...]
st
```

23LC512はCPU native memory mapには存在しません。`sr/sw`はSPI driver経由です。`st`は予約scratch領域の元データを退避し、test後に復元します。

### External I2C EEPROM

```text
er <address> [length]
ew <address> <byte> [byte...]
et [address [length]]
save <ee_addr> <sram_addr> <length>
load <sram_addr> <ee_addr> <length>
memo <text>
```

24FC512はCPU native memory mapには存在せず、software I2C driver経由で操作します。writeは128-byte page境界で分割し、固定delayだけに依存せずACK pollingとtimeoutを使用します。

`et`は最大0x20 bytesを一時上書きし、元データのrestoreと再読出し検証まで成功した場合だけOKを返します。既定範囲は`0xfce0`から16 bytesです。boot counterは`0xfff0..0xfffb`を予約し、一般writeや`et`から保護します。`memo`は`0xfd00`へ最大127 bytesを保存します。

すべての外部memory commandは次を検査します。

```text
address < 0x10000
length <= 0x10000 - address
byte <= 0xff
```

したがって`0x10000`や`0xffff`から2 bytesのアクセスはwrapせず拒否されます。

## GPIO safety policy

GPIO可否はMCUの能力ではなく、Rev0.3 netlist由来の単一`board_pin_t` tableで決定します。不明なpinは拒否します。raw peripheral write、`gpio raw`、`--force`などpolicyを回避する機能はありません。

| Policy | Pins | Generic GPIO operation |
|---|---|---|
| USER | PC0, PC3, PB4, PB8, PB9, PB12, PC14 | 許可。起動時/reset時はinput |
| INPUT_ONLY | PA2/BTN0, PA3/BTN1 | 変更禁止。`button`を使用 |
| BOARD | PA0/LED0、PA1/LED1、PA4-7/SPI、PB3/SD_CS、PB6-7/I2C、PB10-11/UART | generic変更禁止 |
| LOCKED | PB0、PB1、PB5、PC10-11、PC16-19 | generic read/write/mode変更禁止 |

```text
gpio list
gpio status <pin>
gpio read <pin>
gpio mode <pin> input
gpio mode <pin> pullup
gpio mode <pin> pulldown
gpio mode <pin> output <0|1>
gpio write <pin> <0|1>
gpio reset <pin|all>
```

`gpio status`は予約pinでもfunction、policy、拒否理由を表示します。`gpio list`はUSER pinだけを表示します。output化では初期levelを必須とし、output latchを先に設定してからmodeを切り替えてglitchを抑えます。`gpio write`はUSERかつ現在outputのpinだけを許可します。`gpio reset all`が変更するのもUSER pinだけです。

CH32X035のinternal pull-down対応はPA0–PA15とPC16–PC17に限定されます。Rev0.3のUSER GPIOには該当pinがないため、`pulldown`要求は明示的に拒否します。

```text
tiny> gpio status PB0
PB0
function: EXP_PB0
policy: LOCKED
reason: shared with PA7/SPI_MOSI

tiny> gpio mode PB4 output 1
PB4 mode = output, level = 1

tiny> gpio write PB0 1
error: PB0 is locked (shared with PA7/SPI_MOSI)
```

## Software structure

| File | Responsibility |
|---|---|
| `console.c` | USART1、echo、line editing、CR/LF/CRLF |
| `parser.c` | tokenize、数値解析、overflow/range validation |
| `commands.c` | command dispatch、usage/error、各command処理 |
| `pin_policy.c` | hardware非依存のRev0.3 pin policy table |
| `board_gpio.c` | policy通過後のGPIO mode/read/write/reset |
| `spi.c`, `sram_23lc512.c` | SPIと23LC512 |
| `swi2c.c`, `eeprom_24fc512.c` | software I2Cと24FC512 |
| `tiny_monitor.c` | startup、prompt、main loop、heartbeat |

## Planned RISC-V Machine Monitor

次期段階では既存CH32X035 RISC-V Monitorの安全実行基盤を統合します。未実装commandを現在利用可能であるかのように表示しません。

### CPU and machine code

```text
regs
reg <name> [value]
asm <instruction>
word <32-bit-value>
disasm <32-bit-value>
exec <instruction>
step
run
list
clear
reset
```

user instructionは実CPUで実行しますが、U-mode、PMP、trap recoveryを使い、専用scratch RAMとexecution bufferの外へ出られない構成を前提とします。

### Native memory monitor

```text
map
md.b|md.h|md.w <address> [count]
mw.b|mw.h|mw.w <address> <value> [count]
cp.b|cp.h|cp.w <source> <destination> <count>
cmp.b|cmp.h|cmp.w <address1> <address2> <count>
crc32 <address> <length>
```

予定policy:

- 内蔵Flashはread-only
- 専用user scratch RAMだけread/write
- Monitorの`.data/.bss/stack`はアクセス禁止
- 16/32-bit accessはalignment必須
- `cp`は重複範囲でもmemmove相当
- count×access widthのoverflowを事前検査
- SPI SRAMとEEPROMはnative addressへ混在させない
- Flash erase/programは通常の`mw/cp`から実行しない

MMIOはreadでも副作用を持つregisterがあるため、Peripheral全域を無条件には公開しません。公式memory/register mapに基づくread allowlistを設けます。MMIO writeおよびassemblyからのhardware操作は、通常Monitorとは別の明示的な将来のlab modeでboard policyを通す設計とします。

## Tests and verification status

host test対象:

- decimal／hex parser
- invalid inputと32-bit overflow
- `0xffff + 2`、`0x10000` rejection
- CR、LF、CRLF
- USER pin acceptance
- INPUT_ONLY、BOARD、LOCKED rejection
- PB0、PB1/PB5、USB/shared、debug pin保護
- UART、SPI、I2C予約pin保護
- unknown pin、invalid mode/value
- output initial value requirement

firmwareは`-Wall -Wextra -Werror`でビルドします。UART、LED、buttons、GPIO、23LC512、24FC512、boot counter、WCH-LinkE書込みは **HARDWARE TEST REQUIRED** です。実機確認していない項目を動作確認済みとは扱いません。

## Current limitations

- assembler、register context、machine-word executionは未統合
- `md/mw/cp/cmp/map/crc32`は未実装
- MMIO read/writeとlab modeは未実装
- native host GCCがない環境ではhost testはcompile-only確認
- USB、SD filesystem、LCD、GUI、RTOSはv0.1の対象外
