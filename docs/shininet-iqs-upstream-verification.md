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
- Y軸だけを反転するため、IQS listener に ZMK 標準の `zip_xy_transform` と
  `INPUT_TRANSFORM_Y_INVERT` を追加しました。本家ドライバ自体は変更して
  いません。
- `iqs-debug` は `CONFIG_INPUT_IQS9151_LOG_LEVEL=4` だけを有効にします。
  fork 固有の `CONFIG_INPUT_IQS9151_DIAGNOSTIC_LOG` は使用しません。

## 制約

本家ドライバには、fork で追加した IQS用 DYA Studio RPC と runtime設定API
がありません。このブランチでは pointer、gesture、split-input の動作を
検証できますが、IQS固有の DYA Studio WebUIはadvertiseされず、接続も
できない想定です。

## ビルド結果

次の3 targetを `just.sh` と pristine buildで確認しました。

| Target | 結果 | Flash | RAM |
| --- | --- | ---: | ---: |
| `Polaris_R_MODULE_IQS` | 成功 | 230,796 B (28.60%) | 70,484 B (26.89%) |
| `Polaris_R_MODULE_IQS_DEBUG` | 成功 | 274,884 B (34.07%) | 82,644 B (31.53%) |
| `Polaris_R_MODULE_TB` | 成功 | 237,860 B (29.48%) | 68,692 B (26.20%) |

通常版と診断版では、本家 path の `drivers/input/iqs9151.c` が実際に
コンパイルされていることも確認しています。TB buildは、本家moduleを
manifestへ載せた状態で非IQS targetに副作用がないことを確認するために
実行しました。

## 実機確認項目

- IQS9151 が I2C error や RDY timeout なしで初期化される。
- 1本指操作で X/Y が期待する方向へ動く。
- 1本、2本、3本指 gesture が期待するeventを生成する。
- 2本指scrollと慣性が動作し、freezeしない。
- cold boot後に右PeripheralがCentralへ再接続する。
- IQS固有の DYA Studio WebUIが利用不可であることを確認する。
