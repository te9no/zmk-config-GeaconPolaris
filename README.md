# GeaconPolaris ZMK Config
現実とデジタルの境界で、入力を最適化するための設定群です。
- 配列：狭ピッチGRIN風配列
- 対応モジュール：MeKaBuモジュール
　左手はモジュール差し替え可能 (TB/JOY/ENC/TPD)、右手は TB 固定。  
- 電源：有線USB給電 / 単4電池駆動

# Feature
- 左手親指用5方向スイッチ
- 右手人差し指トラックボール

## Signal Map
- `config/Polaris.keymap`: キーマップ定義 (全レイヤーの中枢)
- `boards/shields/GeaconPolaris/`: Shield 定義一式 (回路図の実体)
- `build.yaml`: GitHub Actions のビルドマトリクス
- `config/Polaris.json`, `config/kle.json`: レイアウト/可視化向けデータ

## Build Matrix
`build.yaml` に定義されたターゲットからファームウェアを生成します。  
主要アーティファクト:
- `Polaris_L_MODULE_TB`
- `Polaris_L_MODULE_JOY`
- `Polaris_L_MODULE_ENC`
- `Polaris_L_MODULE_TPD`
- `Polaris_R_MODULE_TB`

## Local Build
ZMK の手順に従い、`seeeduino_xiao_ble` と `Polaris_*` シールドを指定してビルドします。  
組み合わせは `build.yaml` を参照。

## Layers
`config/Polaris.keymap` を参照。  
Def / Fnc / Num /BT

## License
`LICENCE` を参照。

![Keymap](https://github.com/te9no/zmk-config-GeaconPolaris/blob/main/keymap-drawer/Polaris.svg)