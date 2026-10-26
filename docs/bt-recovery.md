# Polaris Bluetooth recovery trial

Branch: `feat/bt-recovery`, based on `zmk-0.4`.

This is an opt-in validation build, not a release or a demonstrated fix for
every idle-time freeze. It preserves the Polaris DYA Studio fork and hardware
configuration. No device has been flashed as part of this integration.

## Pinned components

- cormoran/zmk: `e5c9b6915b56801193e359dd9bad4a167ce0d1b8`
- te9no/zmk-module-bt-recovery: `7e76381d90856a42e739cebf66752afebbe78748`
- Explicit core bridge: `patches/zmk-e5c9b69-bt-recovery.patch`

The bridge was ported from the module's upstream-pinned bridge. The cormoran
relay-event implementation remains intact. Scan-start error handling,
advertisement ownership, connection-to-slot identification and battery event
source bounds were checked against this fork. No existing retry engine was
found in these paths. The installer refuses a different ZMK commit and is
idempotent; it does not overwrite arbitrary changed source files.

## Behavior

The left central retries failed scan starts; the right peripheral retries
failed advertising starts. The retry delays are 1, 2, 4, 8 and 16 seconds,
with a 5-second boot/wake grace. Normal disconnect/reconnection remains owned
by ZMK. Sleep cancels pending recovery. No bonding erase, forced disconnect,
periodic reboot or Bluetooth stack restart is used.

The trial enables diagnostic logging. It does not detect every
connected-but-unresponsive state, a stopped scheduler, or hardware power
fault. A peripheral's last successful data-traffic time is not available.

## Build

Use a separate west workspace, not the configuration repository itself:
the repository has a `zephyr/module.yml` that must not be mixed with a
Zephyr source checkout.

With west, the Zephyr SDK and the ZMK 4.1 build dependencies installed:

```sh
# Run from a directory containing the polaris-bt-recovery checkout.
mkdir -p polaris-bt-west/manifest
cp polaris-bt-recovery/config/west.yml polaris-bt-west/manifest/west.yml
cd polaris-bt-west
west init -l manifest
west update --narrow
python3 ../polaris-bt-recovery/scripts/build-bt-recovery.py Polaris_L_MODULE_LPPS
python3 ../polaris-bt-recovery/scripts/build-bt-recovery.py Polaris_R_MODULE_TB
```

Select any of the eight `artifact-name` values in `build.yaml`.
The script explicitly applies the pinned bridge, then adds `bt-recovery`
to the target's original snippets. UF2 files are written under
`polaris-bt-west/build-bt-recovery/<artifact-name>/zephyr/zmk.uf2`.

The dedicated `bt-recovery.yml` Actions workflow covers all eight targets.
The ordinary firmware publishing workflow is skipped on this trial branch,
so its non-recovery artifacts cannot be confused with trial builds.
`settings_reset` is deliberately excluded: it has no split transport to
recover. Existing release assets and firmware files are not replaced.

For a baseline comparison, use the original `zmk-0.4` firmware. The trial
does not change settings format or request resetting pairings.

## Validation

Native policy tests and tests extracting the actual patched central/peripheral
functions passed locally. These cover bounded retries, duplicate failures,
two peers, foreign/PC connections, invalid source indices and sleep/grace.

Firmware build results are recorded in `bt-recovery-results.md`.
Actions has been prepared but has not been run remotely for this branch.

Before release, test both halves together: normal key input, host reconnect,
split reconnect, sleep/wake, power cycling either half, DYA Studio and
overnight idle with battery power. Hardware fault injection and long-duration
operation remain unverified.
