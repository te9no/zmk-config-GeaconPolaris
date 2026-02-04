# GeaconPolaris ZMK Firmware Configuration

This repository contains the ZMK firmware configuration for the **GeaconPolaris**, a custom-designed ergonomic split keyboard. It's built to offer a highly personalized and efficient input experience, blending a unique physical layout with powerful firmware features.

![Keymap](https://github.com/te9no/zmk-config-GeaconPolaris/blob/main/keymap-drawer/Polaris.svg)

## Features

- **Ergonomic Design**: A split, columnar-staggered layout inspired by the GRIN layout style, designed for comfortable and efficient typing.
- **Modular Left-Hand Side**: The left half supports hot-swappable modules, allowing for different input devices:
    - **TB**: Trackball Module
    - **JOY**: Joystick Module
    - **ENC**: Rotary Encoder Module
    - **TPD**: Touchpad Module
- **Integrated Right-Hand Trackball**: The right half features an integrated trackball under the index finger for pointer control.
- **Advanced Input**: Includes a 5-way switch on the left-thumb cluster for versatile navigation and shortcuts.
- **Controller**: Powered by two **Seeed Studio XIAO BLE** controllers.
- **Power**: Supports both wired USB connection and wireless operation using a single AAA battery.
- **Displays & Lighting**: Integrates an OLED screen (`nice_oled`) for status display and an `rgbled_adapter` for RGB underglow.

## Keymap & Layers

The keymap is defined in `config/Polaris.keymap` and features multiple layers to extend functionality.

- **`DEF` (Default)**: The base layer for standard typing.
- **`FUNC` (Function)**: Provides access to F-keys, navigation, and media controls.
- **`NUM` (Number)**: A number pad and symbol layer.
- **`SNIPE`**: A specialized layer for high-precision mouse pointer movements.
- **`BT` (Bluetooth)**: Manages Bluetooth connections and profiles.
- **`SCROLL` / `SCROLLSNIPE`**: Layers dedicated to scrolling, with and without high-precision mode.

The firmware also utilizes a custom `layout-shift-key-press` behavior to enhance key chording and layer management.

## Project Structure

- `config/Polaris.keymap`: The core keymap definition file.
- `boards/shields/GeaconPolaris/`: Contains all shield definition files, including the base board and modular components (`.overlay`, `.conf`, `.dtsi`).
- `build.yaml`: Defines the build matrix for compiling firmware for each module combination using GitHub Actions.
- `config/kle.json`: Keyboard Layout Editor data for visualization.
- `keymap-drawer/`: Configuration and output for the graphical keymap drawer.

## Building the Firmware

This repository is set up to build firmware automatically using GitHub Actions based on the `build.yaml` matrix. Pre-compiled firmware can be found in the `firmware/` directory of the corresponding branches.

### Build Matrix

The following firmware artifacts are generated:

| Artifact Name         | Left Hand Module | Right Hand Module | Board                | Shields                                             |
| --------------------- | ---------------- | ----------------- | -------------------- | --------------------------------------------------- |
| `Polaris_L_MODULE_TB`   | Trackball        | -                 | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_TB`, `rgbled_adapter`, `nice_oled` |
| `Polaris_L_MODULE_JOY`  | Joystick         | -                 | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_JOY`, `rgbled_adapter`, `nice_oled` |
| `Polaris_L_MODULE_ENC`  | Encoder          | -                 | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_ENC`, `rgbled_adapter`, `nice_oled` |
| `Polaris_L_MODULE_TPD`  | Touchpad         | -                 | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_TPD`, `rgbled_adapter`, `nice_oled` |
| `Polaris_R_MODULE_TB`   | -                | Trackball         | `seeeduino_xiao_ble` | `Polaris_R_Base`, `Polaris_R_TB`, `rgbled_adapter`     |

### Local Build

To build the firmware locally, follow the standard ZMK setup instructions. Use `seeeduino_xiao_ble` as the board and specify the desired shield combination from the build matrix above.

**Example (Left-hand Trackball Module):**
```bash
# west build -b seeeduino_xiao_ble -- -DSHIELD="Polaris_L_Base Polaris_L_TB rgbled_adapter nice_oled"
```

## License

Refer to the `LICENCE` file for details.

---

# GeaconPolaris ZMK ファームウェア設定

このリポジトリは、独自設計のエルゴノミクス分割キーボード **GeaconPolaris** のためのZMKファームウェア設定です。独自の物理レイアウトと強力なファームウェア機能を組み合わせ、高度にパーソナライズされた効率的な入力体験を提供するために構築されています。

![Keymap](https://github.com/te9no/zmk-config-GeaconPolaris/blob/main/keymap-drawer/Polaris.svg)

## 特徴

- **エルゴノミクスデザイン**: GRINレイアウトスタイルにインスパイアされた、快適で効率的なタイピングを目指した分割型カラムスタッガード配列。
- **モジュール式左手側**: 左手側はホットスワップ可能なモジュールに対応しており、異なる入力デバイスを利用できます:
    - **TB**: トラックボールモジュール
    - **JOY**: ジョイスティックモジュール
    - **ENC**: ロータリーエンコーダーモジュール
    - **TPD**: タッチパッドモジュール
- **右手一体型トラックボール**: 右手側には、人差し指で操作するトラックボールが統合されています。
- **高度な入力**: 左手親指クラスターに5方向スイッチを搭載し、多用途なナビゲーションやショートカットを実現。
- **コントローラー**: 2つの **Seeed Studio XIAO BLE** コントローラーで動作。
- **電源**: 有線USB接続と、単4電池1本によるワイヤレス動作の両方をサポート。
- **ディスプレイと照明**: ステータス表示用のOLEDスクリーン (`nice_oled`) と、RGBアンダーグロー用の `rgbled_adapter` を統合。

## キーマップとレイヤー

キーマップは `config/Polaris.keymap` で定義されており、機能を拡張するための複数のレイヤーを備えています。

- **`DEF` (デフォルト)**: 標準タイピング用の基本レイヤー。
- **`FUNC` (ファンクション)**: Fキー、ナビゲーション、メディアコントロールへのアクセスを提供。
- **`NUM` (数字)**: テンキーおよび記号レイヤー。
- **`SNIPE`**: 高精度なマウスポインター操作に特化したレイヤー。
- **`BT` (Bluetooth)**: Bluetooth接続とプロファイルを管理。
- **`SCROLL` / `SCROLLSNIPE`**: 高精度モードの有無を含む、スクロール専用レイヤー。

また、このファームウェアはカスタムの `layout-shift-key-press` ビヘイビアを利用して、キーの組み合わせやレイヤー管理を強化しています。

## プロジェクト構造

- `config/Polaris.keymap`: 中核となるキーマップ定義ファイル。
- `boards/shields/GeaconPolaris/`: ベースボードやモジュール部品を含む、すべてのシールド定義ファイル (`.overlay`, `.conf`, `.dtsi`)。
- `build.yaml`: GitHub Actionsを使用して各モジュール組み合わせのファームウェアをコンパイルするためのビルドマトリクスを定義。
- `config/kle.json`: 可視化のためのKeyboard Layout Editorデータ。
- `keymap-drawer/`: グラフィカルなキーマップ描画ツールの設定と出力。

## ファームウェアのビルド

このリポジトリは、`build.yaml` のマトリクスに基づき、GitHub Actionsを使用してファームウェアを自動的にビルドするように設定されています。コンパイル済みのファームウェアは、対応するブランチの `firmware/` ディレクトリにあります。

### ビルドマトリクス

以下のファームウェアアーティファクトが生成されます:

| アーティファクト名      | 左手モジュール | 右手モジュール | ボード                 | シールド                                            |
| --------------------- | ---------------- | ---------------- | -------------------- | --------------------------------------------------- |
| `Polaris_L_MODULE_TB`   | トラックボール   | -                | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_TB`, `rgbled_adapter`, `nice_oled` |
| `Polaris_L_MODULE_JOY`  | ジョイスティック | -                | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_JOY`, `rgbled_adapter`, `nice_oled` |
| `Polaris_L_MODULE_ENC`  | エンコーダー     | -                | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_ENC`, `rgbled_adapter`, `nice_oled` |
| `Polaris_L_MODULE_TPD`  | タッチパッド     | -                | `seeeduino_xiao_ble` | `Polaris_L_Base`, `Polaris_L_TPD`, `rgbled_adapter`, `nice_oled` |
| `Polaris_R_MODULE_TB`   | -                | トラックボール   | `seeeduino_xiao_ble` | `Polaris_R_Base`, `Polaris_R_TB`, `rgbled_adapter`     |

### ローカルビルド

ファームウェアをローカルでビルドするには、標準のZMKセットアップ手順に従ってください。ボードとして `seeeduino_xiao_ble` を使用し、上記のビルドマトリクスから目的のシールドの組み合わせを指定します。

**例 (左手トラックボールモジュール):**
```bash
# west build -b seeeduino_xiao_ble -- -DSHIELD="Polaris_L_Base Polaris_L_TB rgbled_adapter nice_oled"
```

## ライセンス

詳細は `LICENCE` ファイルを参照してください。
