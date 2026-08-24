# ShiniNet IQS9151 本家ドライバ検証

このブランチでは `te9no/zmk-driver-iqs9151-rpc` を外し、ShiniNet 本家の
リリース版へ戻して Polaris の IQS9151 を検証します。

- Repository: `https://github.com/ShiniNet/zmk-driver-iqs9151`
- Revision: `08a6fd19c5aa5ae7f11daf371b5a391cd8596783` (`v1.0.0`)
- West path: `modules/zmk-driver-iqs9151-upstream`

既存の本家ドライバ checkout には過去のローカル変更が残っていたため、
この検証では別 path に取得しています。これにより、本家 commit 以外の
変更が firmware に混入することを防ぎます。

## 互換調整

- `CONFIG_INPUT_IQS9151_POLLING_ENABLE` を削除しました。本家にはこの設定が
  なく、標準の IRQ 経路で動作します。
- `CONFIG_INPUT_IQS9151_FLIP_Y` を削除しました。本家が公開する向き設定は
  rotation のみです。
- 右 Peripheral の IQS snippet ではなく、split input を受信する左 Central の
  `Polaris_L_interior.dtsi` にある `iqs_listener` へ Y 反転を追加しました。
  `zip_xy_transform` は `REL_X` / `REL_Y` だけを対象とするため、ポインターの
  上下方向だけを反転し、`REL_WHEEL` / `REL_HWHEEL` のスクロール方向は
  変更しません。通常時と low-speed layer の両方に同じ変換を適用します。
- `CONFIG_INPUT_IQS9151_CURSOR_INERTIA_ENABLE=y` とし、1本指のカーソル移動に
  本家ドライバの慣性処理を有効化しました。
- 右側の通常artifactはすべて `zmk-usb-logging` と `cdc-debug-boot` を使用し、
  CDC Debugと1200 baud boot triggerを同じCDC UARTへ統一します。
- 左 Central の `CONFIG_INPUT_IQS9151_STUDIO_RPC` も削除しました。本家には
  該当 Kconfig がなく、残すと Central build が Kconfig warning で停止します。

## 制約

本家ドライバには、fork で追加した IQS用 DYA Studio RPC と runtime設定API
がありません。このブランチでは pointer、gesture、split-input の動作を
検証できますが、IQS固有の DYA Studio WebUIはadvertiseされず、接続も
できない想定です。

## ビルド結果

`build.yaml` の全9 targetを `just.sh` と pristine buildで確認します。

| Target | 結果 | UF2 size |
| --- | --- | ---: |
| `Polaris_L_MODULE_LPPS` | 成功 | 1,004,032 B |
| `Polaris_L_MODULE_TB` | 成功 | 1,021,440 B |
| `Polaris_L_MODULE_JOY` | 成功 | 999,936 B |
| `Polaris_L_MODULE_ENC` | 成功 | 986,624 B |
| `Polaris_L_MODULE_TPD` | 成功 | 990,208 B |
| `Polaris_R_MODULE_TB` | 成功 | 620,032 B |
| `Polaris_R_MODULE_TPD` | 成功 | 581,632 B |
| `Polaris_R_MODULE_IQS` | 成功 | 586,240 B |
| `settings_reset` | 成功 | 112,128 B |

IQS通常版では、本家 path の `drivers/input/iqs9151.c` が実際に
コンパイルされていることも確認しました。左5 targetでは、生成された
DeviceTreeの通常時とlow-speed layerの両方に、X/Y反転を表す
`zip_xy_transform 0x6` が含まれることを確認しました。非IQS targetは、
本家moduleをmanifestへ載せた状態で副作用がないことを確認するために
実行しました。

## 実機確認項目

- 本家版では fork 版よりポインター移動が明確に滑らかになることを確認済み。
- IQS9151 が I2C error なしで初期化される。MCU だけが再起動した場合、
  本家ドライバは software reset 前の非致命な RDY 待ちを timeout することが
  あるため、通常artifactではIQSログをerror以上に限定する。
- 1本指操作で X/Y が期待する方向へ動き、上下方向だけが検証初版から反転する。
- 指を離した後のカーソル慣性が期待どおり動作し、過剰移動しない。
- 1本、2本、3本指 gesture が期待するeventを生成する。
- 2本指scrollの方向が変わらず、慣性が動作してfreezeしない。
- cold boot後に右PeripheralがCentralへ再接続する。
- IQS固有の DYA Studio WebUIが利用不可であることを確認する。
