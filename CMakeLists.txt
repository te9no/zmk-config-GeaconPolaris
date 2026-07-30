zephyr_library()

zephyr_library_sources_ifdef(
  CONFIG_ZMK_BEHAVIOR_ADAFRUIT_BLE_OTA
  src/behavior_adafruit_ble_ota.c
)
