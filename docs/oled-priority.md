# OLED rendering priority

Polaris now uses the same OLED scheduling adjustment that resolved the
Ananta typing/trackball stutter in the reported hardware test.

The left central already uses a dedicated display work queue. Its priority
is changed from the ZMK default of 5 to 12. In this build, input runs at 0,
ZMK BLE at 5 and BT RX at 8; the lower-priority display thread can yield to
these tasks. The display remains enabled with its existing 4096-byte stack.
The right peripheral has no OLED target and needs no display setting.

This change is included on `feat/bt-recovery`; the Bluetooth recovery trial
remains enabled by the dedicated build script. No keymap or sensor scaling
is changed.

## Power settings

- SSD1306 contrast: 128 -> 32 (not a claim of a 75% total power reduction).
- Display task tick: 10ms -> 100ms, reducing periodic display work wakeups.
- Left activity idle timeout: 30s -> 10s. The existing idle handler stops the
  display tick timer and sends DISPLAY_OFF; activity resumes the display.
- Deep sleep remains 30 minutes. Right activity timing remains unchanged.
- Battery status and Bongo Cat stay enabled, with a coarser update cadence.

The left idle timeout is the shared ZMK activity timeout, not a display-only
timer; activity-driven widgets also enter idle earlier. This uses the existing
ZMK idle path rather than disconnecting BLE. The OLED has no configured supply
GPIO, so DISPLAY_OFF does not physically disconnect its supply. Actual current
and runtime improvements need measurement, including while typing, idle with
OLED off, and deep sleep. Readability at contrast 32 also needs hardware review.

Validation (2026-10-28): all eight recovery targets build successfully.
Generated left configurations use the dedicated display queue at priority
12 with the OLED enabled; right configurations keep the display disabled.
Firmware hashes are recorded in `bt-recovery-results.md`.

Polaris hardware has not been flashed or tested for this change. Check BLE
typing while moving the pointing device, OLED updates, and idle/wake on the
actual keyboard before treating its stutter behavior as resolved.
