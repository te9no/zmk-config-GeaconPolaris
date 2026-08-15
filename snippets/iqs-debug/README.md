# IQS diagnostic Snippet

`iqs-debug` enables verbose IQS9151 driver diagnostics. It is intended for
troubleshooting only and is not required by the normal
`Polaris_R_MODULE_IQS` firmware.

The diagnostic build is defined as `Polaris_R_MODULE_IQS_DEBUG` in
`build.yaml`. Use the normal IQS artifact for daily use because debug logging
increases USB serial traffic and can affect timing.
