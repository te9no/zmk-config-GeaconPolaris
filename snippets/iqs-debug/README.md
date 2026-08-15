# IQS upstream log Snippet

`iqs-debug` enables the highest log level exposed by the upstream ShiniNet
IQS9151 driver. It is intended for troubleshooting only and is not required by
the normal `Polaris_R_MODULE_IQS` firmware.

The diagnostic build is defined as `Polaris_R_MODULE_IQS_DEBUG` in
`build.yaml`. Use the normal IQS artifact for daily use because debug logging
increases USB serial traffic and can affect timing.
