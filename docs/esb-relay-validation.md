# Polaris ESB / DYA Studio relay validation

This is an experimental addition on `codex/zmk-0.4-esb-validation`, not a
promotion to `zmk-0.4` or stable `main`. Both halves must be rebuilt and flashed
as a matching pair. A build or host test is not hardware/RF acceptance.

## Enabled paths

- USB Studio on the left central, retaining its local JOY controls and
  runtime macro/combo support.
- Cormoran split relay over the maintained ESB fork on both halves.
- PMW3610 diagnostics on the central, forwarding requests to the right TB.
- Custom-settings split relay and the core settings-RPC module on both halves.
- Independent CDC Debug and count-only relay reception logs on both halves.

The ESB radio payload remains 48 bytes. The relay API data limit is 240 bytes;
larger wire messages are split by the ESB module, not truncated to one packet.
Transport source 0 is the right half; Cormoran Studio exposes it as source 1
(Studio source 0 means the local central). This candidate has one peripheral.

The ZMK, NCS, sensor driver, GPIO 3-wire, ADC/battery configuration, pin release,
USB interface assignment and INFO logging cap remain pinned/unchanged. Only
the ESB module pin, relay configuration and validation observer change.

## Boundaries

- BLE host/controller coexistence is still unavailable in this candidate.
- ESB still lacks encryption, authentication and replay protection. Do not
  use this experimental wireless setup for passwords or sensitive input.
- The transport's existing `ALL_CONNECTED` status is not live discovery.
  There is no link-loss transition on which to rely for automatic stream stop.
  Start with one-shot diagnostics. Continuous frame streaming and disconnect
  recovery remain separate, unaccepted hardware tests; explicitly stop a
  stream before disconnecting.
- Watchdog relay remains disabled: the dedicated ESB shields do not enable
  watchdog providers. Adding the transport does not enable every DYA feature.
- The pinned custom-settings module has its own remote-operation limits;
  generic large-value/chunked writes and every UI control are not promised.
- The earlier right PMW PID `0x00` / OBS `0xff` initialization failure is a
  separate sensor/power/wiring gate. Relay support does not repair that issue.
- Neither firmware has been flashed as part of this implementation.

## Build and generated-artifact audit

Use the workspace `just.sh` (through the existing Docker adapter), as described
in [esb-validation.md](esb-validation.md#build-and-evidence), for both targets:

```sh
bash .zmk-workspace/docker-inside-adapter.bash \
  --profile polaris-esb-validation build-fast ESB --pristine=always
python3 work/polaris-esb/scripts/audit-esb-relay.py
```

Run those commands inside the dev image with the workspace mounted at
`/zmk-workspace`. The audit compares the manifest pin to the actual clean module
checkout, verifies generated DTS/Kconfig, linked relay handlers, CDC routes,
retained INFO/no DBG payload logging, and exported UF2 hashes.

### Results (2026-08-27)

- Firmware source: `64860dde60bf7b20d35bfec0d3d5f61925141be9`.
- Compiled ESB source: `656477caa56d8909ac78e024cbd943caa6aaa7d7`, fetched
  successfully from `https://github.com/te9no/zmk-feature-split-esb` and checked
  out clean in the dedicated profile. The public branch has a later docs-only
  follow-up, `26ea772`; the pinned compiled code is unchanged.
- Host tests: 96 passed with ASan/UBSan, including actual relay serializers,
  both role receive dispatch paths, malformed/reordered/stale fragments,
  240-byte data with 4/32-byte name limits, queue-full retention, inline
  completion and inline-error recovery. Independent review accepted the
  single-peripheral configuration. These are host stubs, not RF emulation.
- [Fork CI](https://github.com/te9no/zmk-feature-split-esb/actions/runs/33066224904):
  successful (96 tests plus detection of the four upstream baseline defects).
- `just.sh ... build-fast ESB --pristine=always`: **2/2 passed** from the
  committed firmware source. Profile log: `build-parallel-20260827-111118`.
- `scripts/audit-esb-relay.py`: **133 assertions passed**, including linked
  relay handlers, generated DTS/Kconfig, correct CDC routing, retained pin
  ownership/ADC configuration and exported UF2 byte equality.
- RAM: left 191,504 / 262,144 bytes (73.05%); right 107,024 bytes (40.83%).
- [Firmware CI](https://github.com/te9no/zmk-config-GeaconPolaris/actions/runs/33066366370)
  is recorded separately from the local build; consult that run for live status.

| Artifact | Bytes | Local UF2 SHA256 |
| --- | ---: | --- |
| `Polaris_L_JOY_ESB_USB` | 799232 | `54d6e10f63ed9160f8f2742464510f420cd1e3e9575d7308f2f3c801ba0c57d2` |
| `Polaris_R_TB_ESB` | 313856 | `63f645422954744fde507aa50e7355248b3e7b523f7608614b58b302fe82cfa5` |

No flash, hardware RPC round-trip, sensor recovery or production acceptance is
implied by these results. The last recorded installed pair remains `72801a6`
with the upstream ESB module, not this new candidate.

## First hardware acceptance (not yet performed)

1. Flash the matching left/right candidates, re-enumerate their actual CDC
   ports, and retain separate boot logs. Verify PMW initialization independently.
2. Connect DYA Studio to the left Studio CDC, not its Debug CDC. Request PMW
   GetInfo / diagnostics for source 1. Confirm the reply is from the right,
   not an empty local-source result.
3. After activity, both Debug ports should report `ESB relay_events=... received`
   at most once per five seconds. These are reassembled-event counts, not
   per-fragment logs or proof of a successful application response. No RPC
   payload, event name, key value or setting value is logged by the observer.
4. Verify the returned sensor identity/CPI and one nonpersistent setting
   round-trip; restore the original value. Exercise keys and pointer alongside
   one-shot RPC and check for stuck keys or pauses.
5. Disconnect the right while a one-shot request is pending. The UI must time
   out, not report success or show an old response. Reconnect and repeat the
   request successfully. The connection icon alone is not evidence.
6. Record results with the exact firmware/module revisions. Leave hardware,
   RPC round-trip and streaming acceptance pending until actually observed.
