# Polaris ZeroShift 開発版

[今回の左右 UF2 を ZIP でダウンロード](firmware/downloads/Polaris-ZeroShift-unified-r01.zip)。同じ ZIP の左右を対応する側に書き込み、設定した側を再起動してください。

左右それぞれに対応する UF2 を一度書き込み、左側を USB または BLE 経由で [ZeroShift Web UI](https://te9no.github.io/zmk-feature-zeroshift/) に接続します。初期状態は左右ともオフです。左右とも PMW3610、エンコーダ、PAT9125（R02 I²C）、LPPS（ADS1220）、1u キー、Cirque タッチパッドを選べます。左側はアナログスティックにも対応します。IQS9151 は右側に加え、実機検証用に左側でも選べます。

1. 左右の電源を入れて「設定する側」を選び、モジュールを選択して保存します。
2. 設定した側の電源を切ってモジュールを取り付け、再起動します。
3. 左側を再起動した場合は再接続し、右側を再起動した場合は再読込して「起動中」の表示を確認します。

### OLED と四連 LED の交換

左手の「左手の表示モジュール」で OLED または四連 LED を選び、「設定を保存」を押します。左手の USB と電池電源を切ってから基板を交換し、再起動して Web UI に再接続してください。

四連 LED は起動時に虹色、続いて左右の接続状態、その後は左2灯で左手、右2灯で右手の電池残量を表示します。左 Cirque は表示ソケットの P1.14 を使用するため、四連 LED と同時には選べません。左 Cirque の使用時は OLED も停止します。

### UF2 のダウンロードと書き込みモード

片側ずつリセットを素早く2回押し、表示されたドライブに左右の対応する UF2 をコピーします。

更新後は各側に1つずつ USB シリアルポートがあります。Web UI を切断してから対象側のポートを 1200 bps・DTR オンで開き、約1秒後に DTR をオフにして閉じると、書き込み用ドライブが現れます。左は通常の Studio 接続にも同じポートを使います。

### 設定と配線

[機種・選択肢の定義](config/zeroshift.json) を親機が共通 Web UI に返します。[左右のビルド設定](build.yaml) は `polaris-zs-left` と `polaris-zs-right` を使います。各モジュールの配線は[シールドのデバイスツリー定義](boards/shields/GeaconPolaris/modules/)にあり、起動時に選んだドライバーが初期化されます。

[通常のキーマップと操作説明（従来版）](https://github.com/te9no/zmk-config-GeaconPolaris/blob/8095c7f/README.md#keymap)
