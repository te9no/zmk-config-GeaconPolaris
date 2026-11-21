# Local recovery build results

Validated on 2026-10-28. No device flashing or hardware recovery test was performed.

| Target | UF2 bytes | SHA-256 |
| --- | ---: | --- |
| Polaris_L_MODULE_LPPS | 1006592 | `8eed02b477104c29ff9a299f99ab506184fa969bf985c789ac3dbf12b757e739` |
| Polaris_L_MODULE_TB | 1024000 | `bf5c37728518ffb83b4a7c5cd915b8a30f629711c752c33d60e0b8390ded0fdf` |
| Polaris_L_MODULE_JOY | 1002496 | `4677f4cd5b40402af4c0bd9f7d56ab0eed82da35038d64c56e41d7d4657632f1` |
| Polaris_L_MODULE_ENC | 989184 | `84cbaa4481da6318c0485c127fb7b24ec086736847ab13f54b9412e4ca0b5945` |
| Polaris_L_MODULE_TPD | 992768 | `98d2858a11168c2e75739c326316fcd5ab3604132da94d16b403ae84de241152` |
| Polaris_R_MODULE_TB | 619520 | `75718e07208aa53d79ca8eeaeff7e978cefc5abc9539df500088241ac8a2bc56` |
| Polaris_R_MODULE_TPD | 584704 | `aaeda84269baf78bcee110481fa8c23e80475a85ff45893ff4f3f7fefedec59f` |
| Polaris_R_MODULE_IQS | 588800 | `f32dabb7d751bc1c7cdcb22c2cc3a13dc24fb4fb826e72581656a85b14c62f52` |

All eight configurations contain the enabled recovery module, bridge and diagnostics; left/central and right/peripheral roles were checked. Every UF2 block header/footer was validated.

Passed: native policy tests; actual cormoran owner-function mock tests; clean patch application; repeated application; wrong-revision rejection.

Built locally in zmk-workspace-dev:latest with dependencies resolved from this branch's pinned manifest, cormoran ZMK e5c9b691 and Zephyr 4.1. The earlier recovery-only commit passed GitHub Actions; this OLED priority/power update has been validated locally, not yet remotely. settings_reset is not a recovery target.

Unverified: real BLE failure/recovery, sleep/wake, DYA Studio behavior after flashing, power use and long-duration idle stability.

Artifacts: ../polaris-bt-west/build-bt-recovery/<target>/zephyr/zmk.uf2. Build logs: ../polaris-bt-west/<target>.log.

Left OLED configuration verified: enabled, dedicated queue priority 12, stack 4096, contrast 32, tick 100ms, idle blanking at 10s, deep sleep 30min. Right OLED remains disabled. Power reduction and readability are not measured.
