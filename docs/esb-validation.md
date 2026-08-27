# Polaris ESB functional trial

This branch is based on source `84217286bbc5d064b1d6a3e3b3f671017e687942`.
It is not merged into `zmk-0.4` or `main`. No additional dongle is required:

| Artifact | Role | Host connection | CDC ports |
| --- | --- | --- | --- |
| `Polaris_L_JOY_ESB_USB` | USB central, left JOY, standard OLED | USB HID + local Studio | board CDC = Debug; snippet CDC = Studio/1200-baud boot trigger |
| `Polaris_R_TB_ESB` | ESB peripheral 0, right PMW3610 | ESB to the left; USB for debug/power | logging CDC = Debug/1200-baud boot trigger; board CDC disabled |

The normal BLE hardware overlays/configurations are retained, but adding NCS to
the manifest changes the build environment. A pristine build of the old
`Polaris_L_MODULE_JOY` in this profile fails in NCS security CMake (missing `pk.c`,
`platform.c`, `oberon_helpers.c`). **Build normal BLE firmware from its original
branch and profile.** The trial branch's `build.yaml` and Actions matrix contain
only the two ESB targets. The original nine targets remain in the source base
branch; this is not a claim that all eleven targets are compatible.

## Safety and scope

The [ESB module](https://github.com/badjeff/zmk-feature-split-esb) sends raw events
with CRC32, without encryption, authentication, or replay protection. The unique
Polaris radio addresses prevent accidental default-address collisions; they are
not a security boundary. Use only harmless test keys, never passwords or other
sensitive input. This is a functional trial, not a latency benchmark or production
wireless configuration. Logging changes timing; no 1 ms result is claimed.
The trial now sets `CONFIG_LOG_MAX_LEVEL=3` (INFO) to compile out all DBG logging,
including key-value logs: USB logging otherwise makes the numeric ZMK log level 4
even with `CONFIG_ZMK_LOG_LEVEL_INF=y`. Aggregate event counts, battery reports,
and PMW initialization messages remain available at INFO. The initial `ea2ca2c`
build lacked this global cap and is superseded for input testing.

Both halves must use these matching trial artifacts. Reverting means flashing the
matching normal BLE pair. Persistent settings are not erased by these artifacts.
The right PMW3610 previously returned PID `0x00`/OBS `0xff` with both new and old
BLE firmware; this is a separate unresolved sensor/power/wiring investigation.
Key-position and battery events can test ESB even if PMW initialization fails.

## Maintained ESB fork (2026-08-27)

The trial now pins
[`te9no/zmk-feature-split-esb@58c8f91`](https://github.com/te9no/zmk-feature-split-esb/commit/58c8f912dae87b8197c4d6229e3f2df8cc52daaf).
It fixes packet/retry identity, preserves retry ordering, exits incomplete-frame
receive loops, accepts valid small commands with type/length validation, and
propagates radio initialization errors. The fork's 87 host regression cases pass
with ASan/UBSan; all four baseline defects are detected by the same suite.
Those tests do not validate RF, real interrupt concurrency or MPSL timeslots.

Only the ESB module URL/revision changes in this firmware candidate. ZMK/NCS,
hardware pins, CDC logging and INFO privacy cap remain as before. This adoption
does not flash the connected keyboard: the previous `72801a6` pair remains its
last recorded installed firmware. Its right PMW initialization failure remains
an independent unresolved hardware gate. No stable-branch merge is implied.

Before building, run the existing wrapper command below with `update` instead
of `build-fast ...` to fetch the pinned fork. Keep generated-build and hardware
results separate in the [Fleet trial ledger](https://te9no.github.io/zmk-shield-fleet/).

## Pinned dependencies

| Dependency | Revision |
| --- | --- |
| `cormoran/zmk` (unchanged) | `e5c9b6915b56801193e359dd9bad4a167ce0d1b8` |
| `te9no/zmk-feature-split-esb` | `58c8f912dae87b8197c4d6229e3f2df8cc52daaf` |
| `badjeff/sdk-nrf`, `v3.1-branch+zmk-fixes` | `9b3d2623fdcd9c0fd0284f860beea924568c9826` |
| `nrfconnect/sdk-nrfxlib`, `v3.1-branch` | `dfadf17305d8f000eda9aa74a5b9ff1c5647a23e` |

The manifest pins all three newly added dependencies. The existing Cormoran
PMW3610 driver, analog/oversampling modules and GPIO 3-wire driver are retained.
No dependency source is patched by this configuration. The upstream ESB CMake
itself adjusts the cloned ZMK `split/central.h` peripheral-count macro; this
automatic build-time edit is confined to this dedicated profile. Do not reuse
the profile for ordinary BLE builds.

Peripheral ID is **0**, count **1**, matching the current ESB source enumerator
and core/OLED battery slot 0. Radio base addresses are `50 4f 4c 41` and
`52 49 53 27`, prefixes `91 26 53 a4 75 c6 37 e8`. Hardware ACK and CRC are enabled.
The ESB payload remains 48 bytes and queue sizes are bounded for nRF52840 RAM.

## Trial-only differences

| Feature | Trial behavior / reason |
| --- | --- |
| BLE host/split and BLE management | Disabled; this ZMK 0.4 ESB setup cannot share the BLE controller. The old BT layer bindings become transparent except existing arrow keys. |
| Bongo Cat | Standard built-in OLED on the left; the pinned Bongo module calls BLE APIs unconditionally. USB and battery widgets remain. |
| RGB LED peripheral battery cycling | Self battery only; the module's peripheral mode uses a BLE-only count macro. Core peripheral battery fetching/proxy remains enabled. |
| Cormoran split relay / remote PMW Studio RPC | Disabled; ESB does not implement relay/heartbeat message variants or fragmentation. Raising payload size alone is not compatible. |
| `settings-rpc` module | Disabled; its activity-report source uses relay macros unconditionally even when split relay is off. |
| Local Studio, JOY runtime controls, macro/combo | Retained on the USB central; build verified, UI/hardware still requires a test. No remote sensor diagnostic result is promised. |
| JOY ADC/oversampling, PMW GPIO 3-wire, XIAO pin release, NiMH battery definition | Kept from the source base; not changed to make ESB work. |

## Build and evidence

Use the workspace's unchanged `just.sh` via its Docker adapter. Dedicated profile:
`polaris-esb-validation`. Example from the workspace root:

```sh
docker run --rm --user 1000:1000 \
  --env ZMK_CONFIG_NAME=zmk-config-GeaconPolaris \
  --env ZMK_CONFIG_BRANCH=codex/zmk-0.4-esb-validation \
  --volume "$PWD:/zmk-workspace" --workdir /zmk-workspace \
  zmk-workspace-dev:latest \
  bash .zmk-workspace/docker-inside-adapter.bash \
  --profile polaris-esb-validation build-fast ESB --pristine=always
```

Initial setup uses the same wrapper with `init work/polaris-esb` in place of
`build-fast ...`. Keep the explicit output-name overrides because worktree git
metadata may not resolve from within the container.

### Historical upstream-module build (`72801a6`, not the new fork)

Pristine trial build: **2/2 successful**, log directory
`.zmk-workspace/profiles/polaris-esb-validation/logs/build-parallel-20260827-061826/`.
Initial failure and the BLE compatibility smoke are retained separately in
`build-esb-attempt1.log` and `build-ble-joy-smoke.log`.
Generated DTS, `.config`, ELF and UF2 are in that profile's `build/<artifact>/zephyr/`.
Copied UF2 output is
`firmware/zmk-config-GeaconPolaris/codex-zmk-0.4-esb-validation/`.
The profile's `logs/check-esb.py` passes **112 assertions** against the two
generated DTS/configurations, pinned manifest, two-target matrix and copied UF2.
This includes `LOG_MAX_LEVEL=3`, `LOG_OVERRIDE_LEVEL=0`, absent DBG key/mouse
format strings in `zmk.bin`, and retained INFO count/battery/PMW init strings.
Its result is saved in `logs/esb-audit-infocap-summary.txt`. UF2 SHA256 values:

- Left: `1466087130461fbc14152dc3537beb4c55cba56a22a79922caf791d644c5d74a`
- Right: `05256070eb1d51db21ac9ce863fbb6d2c12f86a5d28fd5be098889460eae07e8`

No flash or hardware acceptance is implied by a successful build.

## CDC functional acceptance

1. Flash the matched pair; enumerate the physical ports again instead of assuming
   old COM numbers. Left Debug is board CDC and left boot trigger is Studio CDC.
   Right Debug is the only active CDC and is also its boot trigger.
2. Check boot logs independently of sensor initialization. With the right awake,
   press harmless test keys. Left logs aggregate `ESB source=0 position_events=...`
   at most once per five seconds after activity. No key position/keycode/state or
   payload is logged by this observer.
3. Left logs `ESB source=0 battery=... received` when a battery event arrives.
   Battery events depend on state changes; a constant charge reading can make
   them infrequent. Do not infer radio failure solely from an absent battery log.
4. Disconnect/reconnect the right and verify counts stop/resume. The upstream
   transport status is always `ALL_CONNECTED`, so its connection icon is **not**
   evidence of a live link. Test right pointer separately if the sensor starts.
5. The observer only consumes public ZMK events; it does not hook radio callbacks.
   Position/battery counts prove central receipt, not direct TX/ACK counters.
   ACK is configured but not directly observed by these logs. Firmware may log
   radio errors; no claim of measured packet loss or latency is made.

Hardware checks (input, battery receipt, USB Studio, OLED) remain pending.
