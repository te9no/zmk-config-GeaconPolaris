# Local recovery build results

Validated on 2026-10-26. No device flashing or hardware recovery test was performed.

| Target | UF2 bytes | SHA-256 |
| --- | ---: | --- |
| Polaris_L_MODULE_LPPS | 1006592 | `0a36eb05feb6b62a3c08d84c364dadaa41c271f42901f3c1bb71481e43ce7fbe` |
| Polaris_L_MODULE_TB | 1024000 | `53457a0863e387a33acf5d9abd27562c1e34701d892288fdd673f3f887f1e82f` |
| Polaris_L_MODULE_JOY | 1002496 | `d7150b95ae23d52d4f04c58a3bf31a7cf63e489c0c2cde3e797b5b4a846b7ca2` |
| Polaris_L_MODULE_ENC | 989184 | `11ec4e18adec6c3b07152bb74e01ae413060462429157a073d6beab9d8f9a608` |
| Polaris_L_MODULE_TPD | 992768 | `fa44dfbfb3917332c04475052e08623ce809535b75c5bc2a31258629f8d20d2c` |
| Polaris_R_MODULE_TB | 619520 | `75718e07208aa53d79ca8eeaeff7e978cefc5abc9539df500088241ac8a2bc56` |
| Polaris_R_MODULE_TPD | 584704 | `aaeda84269baf78bcee110481fa8c23e80475a85ff45893ff4f3f7fefedec59f` |
| Polaris_R_MODULE_IQS | 588800 | `f32dabb7d751bc1c7cdcb22c2cc3a13dc24fb4fb826e72581656a85b14c62f52` |

All eight configurations contain the enabled recovery module, bridge and diagnostics; left/central and right/peripheral roles were checked. Every UF2 block header/footer was validated.

Passed: native policy tests; actual cormoran owner-function mock tests; clean patch application; repeated application; wrong-revision rejection.

Built locally in zmk-workspace-dev:latest with dependencies resolved from this branch's pinned manifest, cormoran ZMK e5c9b691 and Zephyr 4.1. GitHub Actions is prepared but not remotely executed. settings_reset is not a recovery target.

Unverified: real BLE failure/recovery, sleep/wake, DYA Studio behavior after flashing, power use and long-duration idle stability.

Artifacts: ../polaris-bt-west/build-bt-recovery/<target>/zephyr/zmk.uf2. Build logs: ../polaris-bt-west/<target>.log.
