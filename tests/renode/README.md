# Polaris のモジュール保存試験

ワークスペース直下で実行します。プロファイルをこのブランチから初期化し、Polaris 対応の Web UI ブランチの studio-web で npm ci と npm run build を実行してください。

```sh
ZMK_CONFIG_ROOT=work/polaris-zeroshift ZMK_CONFIG_NAME=Polaris ZMK_BUILD_MATRIX=build-renode.yaml ZMK_FIRMWARE_TEST_PURPOSE=renode-zeroshift-r01 ./just.sh --profile polaris-zeroshift build-parallel Polaris-ZeroShift-sim 2
ZEROSHIFT_SHARED_RPC="$PWD/work/polaris-zeroshift-ui/studio-web/dist/rpc.mjs" python3 -B work/polaris-zeroshift/tests/renode/test_studio.py --renode .zmk-workspace/tools/renode-1.17.0/renode --firmware-dir firmware/testing/Polaris/renode-zeroshift-r01 --log /tmp/polaris-renode.log
```

仮想 UART で Web UI と同じ Studio クライアントを接続し、左右の保存値と再起動後の起動値を照合します。IQS9151 と Cirque は初期化用の I²C 応答、スティックは固定 ADC 値を使用。PMW3610 の非同期セルフテスト、物理入力、USB 列挙と BLE の通信は実機検証事項です。
