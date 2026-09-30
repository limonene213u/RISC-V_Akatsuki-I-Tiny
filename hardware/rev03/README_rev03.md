# AKATSUKI I TINY Rev0.3 (THT エディション)

**全部手はんだで作れる版です。JLC にはプリント基板だけ注文します（部品実装なし）。**

## JLC への注文
1. `manufacturing/gerber_rev03_JLC.zip` を JLC にアップロード（GitHubへ手動アップロード後）
2. 基板サイズ 100×70mm、2層、厚さ 1.6mm。色はお好みで（黒ならシルクは白）
3. **「PCB Assembly」はオフのまま**（ここをオンにすると高くなります）

## 部品
`parts_list_rev03.csv` を見てください。秋月でほぼ全部そろいます。
- USB-C は **秋月のDIP化キット (115426)** を使います。VBUS（四角パッド）が左、コネクタは基板の上端向き
- U5（ESD保護 IC）は表面実装なので **付けなくてOK**。付けなくても配線はつながっています

## はんだ付けのコツ
- 背の低い部品から：抵抗 → セラコン → ICソケット → LED/スイッチ → 電解コン → 大物
- 向きがあるもの：LED（短い足＝四角パッド）、電解コン（＋印）、ICソケット（切り欠き）、TA48033S（1=IN 2=GND 3=OUT をデータシートで確認）
- マイコンモジュール U1 はピンソケットに挿す形がおすすめ

## 使い方メモ
- **SW4 (MAIN PWR)**：電源スイッチ。このチップにはリセットピンが無いので、リセットしたいときは OFF→ON
- **SW3 (HOST PWR)**：USBホスト側への 5V。**マイコンが起動してから ON** にしてください
- シルクの OFF/ON は念のためテスターで確認してから使うと安心です
- JP1 は普段ジャンパなし（USB-C を挿さずに WCH-Link から給電するときだけ付ける）

## ファイル
- `manufacturing/gerber_rev03_JLC.zip` … JLC に出すファイル（GitHubへのbinary upload待ち）
- `source/akatsuki_tiny_rev03.zip` … 元のRev0.3設計一式（GitHubへのbinary upload待ち）
- `parts_list_rev03.csv` … 部品表
- `netlist_rev03.csv` … 接続表
- `../../docs/BOM_rev03.md` … 実装用BOM
- `../../docs/PINOUT_rev03.md` … ピン表
- `../../docs/BRINGUP_rev03.md` … 初回bring-upチェックリスト

元のRev0.3パッケージでのチェック結果：ERC エラー 0、DRC 未接続 0（残りはシルクの重なり警告だけ）。


## Binary archive integrity

GitHubへ手動アップロードする元ファイル:

- `gerber_rev03_JLC.zip`: 161,207 bytes  
  SHA-256 `fbb4d96957f28013e82998b69a4798d8ce7ebaeb7e506b686b80bedc80d15ccd`
- `akatsuki_tiny_rev03.zip`: 7,618,559 bytes  
  SHA-256 `3f05fe586889200f3471e59771c409332d25a38a4daf7a1f5751d7d55efa80fc`

古いRev0.2 archiveと混同しないこと。
