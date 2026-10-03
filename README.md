# Akatsuki I Tiny

Akatsuki I Tinyは、WCH **CH32X035G8U6（QingKe V4C / RV32IMAC）**を使った、小型のRISC-V学習・実験用コンピュータです。

UART端末からLED、GPIO、SPI SRAM、I2C EEPROMを操作できるだけでなく、将来的には機械語、RISC-Vアセンブリ、レジスタ、load/store、実アドレス、MMIOを実機上で順番に学べる **RISC-V Machine Monitor** を目指しています。

このリポジトリを、Akatsuki I Tinyのハードウェア、ファームウェア、製造資料、bring-up資料の正本とします。

> [!NOTE]
> [`akatsuki-i`](https://github.com/limonene213u/akatsuki-i) は、RISC-V CPUそのものを設計・開発するためのPIC32＋FPGA基板であり、本プロジェクトとは別物です。

## 現在の状態

| 項目 | 状態 |
| --- | --- |
| 現行基板 | Rev0.3 THT Edition |
| MCU | CH32X035G8U6 |
| Monitor | Tiny Monitor v0.1 |
| Firmware build | MounRiver Studio 2付属toolchainで確認済み |
| 実機確認 | 継続中。未確認項目は各文書で明記 |
| KiCad metadata | 収録済み |
| Gerber／完全なKiCad source archive | 元ファイルの手動アップロード待ち |

Rev0.3は、できるだけ手はんだで組み立てられるTHT中心の構成です。基板だけをJLCPCB等へ発注し、部品は自分で実装することを想定しています。

## この基板でできること

- CH32X035G8U6上でRISC-V firmwareを実行
- WCH-LinkEによる書き込みとデバッグ
- 115200 baudのUART console
- 2個のLEDと2個のbutton
- board policyで保護されたユーザーGPIO
- 23LC512 SPI SRAM（64 KiB）
- 24FC512 I2C EEPROM（64 KiB）
- SPI／SD、USB、I2C、GPIOの拡張実験
- 将来の機械語・assembly・register・native memory学習

## リポジトリ構成

```text
RISC-V_Akatsuki-I-Tiny/
├── README.md
├── docs/
│   ├── BOM_rev03.md          Rev0.3実装用BOM
│   ├── BRINGUP_rev03.md      初回通電・bring-up手順
│   └── PINOUT_rev03.md       pin assignmentと保護条件
├── hardware/
│   └── rev03/
│       ├── README_rev03.md   製造・実装時の注意
│       ├── netlist_rev03.csv 配線のsource of truth
│       ├── parts_list_rev03.csv
│       └── kicad/            KiCad project metadata
└── firmware/
    └── tiny-monitor/         MounRiver Studio用Monitor firmware
```

Rev0.3の配線については、[`hardware/rev03/netlist_rev03.csv`](hardware/rev03/netlist_rev03.csv) をsource of truthとします。README、pin定義、firmwareとの矛盾を見つけた場合は、推測で配線を決めずに確認してください。

MCUの物理address、memory容量、peripheral mapについては、[CH32X035 Datasheet V1.7](https://akizukidenshi.com/goodsaffix/CH32X035.pdf) を一次資料とします。

## Tiny Monitor v0.1

[`firmware/tiny-monitor/`](firmware/tiny-monitor/) には、Rev0.3専用のbare-metal UART monitorが入っています。WCH EVT／MounRiver Studio系のstartup、linker script、StdPeriphを使用し、Arduino、RTOS、`ch32fun`には依存しません。

現在実装されている主な機能は次の通りです。

- firmware echo付きUART console
- CR、LF、CRLF、Backspace、Deleteの処理
- `usage:`／`error:`形式の入力エラー表示
- board-level safety policy付きGPIO操作
- LED／button操作
- SPI SRAMのread、write、test
- I2C EEPROMのread、write、test
- SRAMとEEPROM間のsave／load
- EEPROM memoとboot counter
- 64 KiB境界、byte値、予約領域の検査
- datasheet準拠のphysical memory map表示
- USER SCRATCHに限定したnative memory操作、比較、CRC-32

詳細なcommand仕様、安全設計、制限事項は、[`firmware/tiny-monitor/README.md`](firmware/tiny-monitor/README.md) を参照してください。

### UART terminal設定

| 項目 | 設定 |
| --- | --- |
| Baud rate | 115200 |
| Data | 8 bit |
| Parity | none |
| Stop bit | 1 |
| Flow control | none |
| Local echo | off |

接続は3.3 V TTLです。5 V UART信号を接続しないでください。

| J5 | USB-UART adapter |
| --- | --- |
| J5-1 GND | GND |
| J5-2 UART_TX | RX |
| J5-3 UART_RX | TX |

起動すると、次のpromptが表示されます。

```text
Akatsuki I Tiny Monitor
Rev0.3
SRAM: OK
EEPROM: DETECTED
Boot count: 42
Type 'help' for commands.

tiny>
```

`EEPROM: DETECTED`は、I2C address `0x50`からACKを受信したことを示します。EEPROM全体のread/write試験に成功した、という意味ではありません。

### Command概要

```text
help
info
pins
map [ram|flash|mmio|policy]

md.b|md.h|md.w <address> [count]
mw.b|mw.h|mw.w <address> <value> [count]
cp.b|cp.h|cp.w <source> <destination> <count>
cmp.b|cmp.h|cmp.w <address1> <address2> <count>
crc32 <address> <length>

gpio list
gpio status <pin>
gpio read <pin>
gpio mode <pin> input|pullup|pulldown
gpio mode <pin> output <0|1>
gpio write <pin> <0|1>
gpio reset <pin|all>

led 0 on|off|toggle|auto
button <0|1>

sr <address> [length]
sw <address> <byte> [byte...]
st

er <address> [length]
ew <address> <byte> [byte...]
et [address [length]]

save <ee_addr> <sram_addr> <length>
load <sram_addr> <ee_addr> <length>
memo <text>
```

数値は通常表記なら10進、`0x` prefix付きなら16進として扱います。

```text
tiny> gpio status PB4
tiny> gpio mode PB4 output 1
tiny> gpio write PB4 0
tiny> sr 0x1000 16
tiny> ew 0x2000 0x41 0x42 0x43
```

## GPIO safety policy

GPIOとして使えるかどうかは、MCU単体の機能ではなく、Rev0.3の基板配線に基づくpin policy tableで決定します。不明なpinは許可しません。

| Policy | Pins | 扱い |
| --- | --- | --- |
| USER | PC0, PC3, PB4, PB8, PB9, PB12, PC14 | generic GPIO commandで操作可能。初期状態はinput |
| INPUT_ONLY | PA2, PA3 | button入力。output化禁止 |
| BOARD | PA0, PA1, PA4–PA7, PB3, PB6, PB7, PB10, PB11 | LED、SPI、SD、I2C、UART専用 |
| LOCKED | PB0, PB1, PB5, PC10, PC11, PC16–PC19 | generic GPIO操作禁止 |

特に次のpinには注意してください。

- **PB1／PB5:** MCU内部で短絡されているため、絶対にoutputへ設定しない
- **PB0／J7-5:** PA7／SPI_MOSIとの内部共有条件があるため、外部から駆動しない
- **PC16／PC17:** USB DM／DP。PC11／PC10との内部共有あり
- **PC18／PC19:** WCH-Linkのdebug/programming interface
- **PB10／PB11:** Monitor自身が使用するUART
- **PA5／PA6／PA7:** 23LC512と共有するSPI bus
- **PB6／PB7:** 24FC512が使用するsoftware I2C bus

初期版には、raw GPIO register write、`--force`、任意MMIO writeなど、policyを回避するcommandはありません。

## Build

### Command line

MounRiver Studio 2付属のGNU MakeとRISC-V Embedded GCCを使用します。PowerShellではリポジトリのルートから次を実行できます。

```powershell
& 'C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\others\Build_Tools\Make\bin\make.exe' `
  -C firmware/tiny-monitor -j16 all
```

生成物は`firmware/tiny-monitor/build/`に出力されます。

```text
tiny-monitor.elf
tiny-monitor.hex
tiny-monitor.bin
tiny-monitor.map
```

### MounRiver Studio

1. `firmware/tiny-monitor/`をprojectとして開く
2. 外部でファイルを変更した場合はprojectをRefresh
3. `Project → Clean...`を実行
4. Buildする
5. WCH-LinkEからELFまたはHEXを書き込む

`tests/`にはhost用unit testがあり、firmware本体のリンク対象ではありません。以前のような`multiple definition of main`が出る場合は、`tests/`がtarget buildへ混入していないか確認してください。

### WCH-LinkE

| J4 | WCH-LinkE |
| --- | --- |
| J4-2 GND | GND |
| J4-3 DBG_DIO | DIO |
| J4-4 DBG_DCK | DCK |

WCH-LinkEから給電するときだけJP1を接続します。別電源とWCH-LinkEの同時給電は避け、GNDを共通にしてください。

## Rev0.3の組み立てとbring-up

組み立て前に次の資料を確認してください。

- [Rev0.3製造・実装メモ](hardware/rev03/README_rev03.md)
- [実装用BOM](docs/BOM_rev03.md)
- [Pinout](docs/PINOUT_rev03.md)
- [Bring-up checklist](docs/BRINGUP_rev03.md)

推奨する確認順序は次の通りです。

1. +3.3 V／GNDと+5 V／GNDの短絡確認
2. 電源回路の確認
3. PC19／PC18でWCH-Linkをbring-up
4. PB10／PB11でUARTをbring-up
5. Tiny Monitorを書き込み、起動表示を確認
6. LED、button、USER GPIOを確認
7. SPI SRAMを確認
8. software I2CとEEPROMを確認
9. USB／SDを最後に確認

## 設計資料とbinary archive

テキスト形式のBOM、netlist、pinout、bring-up資料とKiCad metadataは収録済みです。以下のbinary archiveは、元ファイルを取得して手動でアップロードする必要があります。

| File | Size | SHA-256 |
| --- | ---: | --- |
| `hardware/rev03/manufacturing/gerber_rev03_JLC.zip` | 161,207 bytes | `fbb4d96957f28013e82998b69a4798d8ce7ebaeb7e506b686b80bedc80d15ccd` |
| `hardware/rev03/source/akatsuki_tiny_rev03.zip` | 7,618,559 bytes | `3f05fe586889200f3471e59771c409332d25a38a4daf7a1f5751d7d55efa80fc` |

古いRev0.2 archiveで代用しないでください。

## 今後の方向

GPIO commandは、Lチカやdigital sensor入力を簡単に試すための入口です。Akatsuki I Tinyの本命は、その下でCPUが行っている処理を実機で理解することです。

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

`map`、`md`、`mw`、`cp`、`cmp`、`crc32`は、安全なmemory policy付きで実装済みです。今後は`regs`、`reg`、`asm`、`word`、`disasm`、`exec`、`step`、`run`などを、U-mode、PMP、trap recoveryと組み合わせて追加する予定です。

これらは現時点では未実装です。現在利用できるcommandと将来計画を混同しないよう、実装完了後にhelpとREADMEを同時に更新します。

## 関連プロジェクト

- [CH32X035 RISC-V Monitor](https://github.com/limonene213u/risc-v_CH32X035G8U6) — console、parser、commands、安全なCPU命令実行環境の参考実装
- [Akatsuki I](https://github.com/limonene213u/akatsuki-i) — RISC-V CPUそのものを設計・開発するための別プロジェクト

## Project management

- [Akatsuki I Tiny — Linear project](https://linear.app/aets-magi/project/akatsuki-i-tiny-083b4a1547ce)
- [Akatsuki I Tiny 実装表／BOM](https://linear.app/aets-magi/document/akatsuki-i-tiny-%E5%AE%9F%E8%A3%85%E8%A1%A8bom-8386743a65c7)
