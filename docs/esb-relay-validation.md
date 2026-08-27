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

Results will be recorded here after the candidate passes its build gates.

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
