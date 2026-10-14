# Calibration, schedules and daily use

## First calibration

1. Keep schedules disabled. Confirm motor direction and encoder counts with the motor detached from the chain.
2. Attach the chain and secure the unit. Open Calibration. Hold the jog control and release to stop. Keep watching the blind.
3. Jog to the fully raised position without forcing the mechanical stop. Release, wait until stopped, and save the upper limit.
4. Jog to the fully lowered position. Release and save the lower limit, then finish/save calibration.
5. Test Raise, Lower and Stop. At upper, another Raise is rejected; at lower, another Lower is rejected.

Reverse motor in Settings if needed, then recalibrate. A fault during calibration cancels calibration; save both limits afresh. Every continuous run is limited to 60 seconds; the setting may shorten, but not lengthen, that cap. Hold-to-jog controls also stop on release/lost heartbeat. A new command does not bypass the runtime limit.

## After a reboot or power loss

From firmware 2.0.4, a valid last stopped position is restored automatically after restart, provided it matches the saved calibration. You do not need to press Recover position after a normal power cycle. This assumes the beads and blind have not moved while power was off. The raw encoder counter restarts at zero, but an offset preserves the saved logical position.

If power was lost during movement, the saved position is invalid/missing, or calibration changed, position remains unknown and automatic travel/schedules stay blocked. Supervise the blind to its saved upper reference and confirm it, or recalibrate. Only confirm the upper reference when physically at that same saved position. If the chain moved manually or slipped while off, recover/recalibrate even if the displayed saved position looks valid. Schedules resume after valid NTP time; elapsed events are not replayed.

## Schedules

The browser can be closed; rules run on the ESP. Set the device clock via NTP and check it displays valid UK time. For sunrise/sunset, enter latitude/longitude and enable use of that location. Add up to eight rules with weekdays, fixed time or sunrise/sunset, offset and target percentage. Save and enable schedules.

Times use **24-hour UK local time**. `02:45` means 2:45 AM; `14:45` means 2:45 PM. A future eligible event can run today. If its time has passed, the next eligible date is shown. From firmware 2.0.3, editing a rule creates a fresh schedule identity: its next future occurrence can run today even if the earlier version was already executed or skipped. Unchanged rules remain protected from duplicate execution. Saving an elapsed time never replays it immediately.

Automatic GMT/BST changes are handled on-device. Once NTP synchronises, the clock can continue through a network outage for up to 24 hours since last sync. Missed events, large time corrections, busy/calibrating/faulted/unknown-position events are skipped rather than replayed. An event already recorded is not automatically repeated after a reboot.

Test a rule a few minutes in the future while supervising. Check both the next-action time and that the intended weekday/target are correct. No sun event is calculated until location is configured.

## Multiple blinds

Install the same firmware on each controller, then calibrate and schedule each independently. Choose one to show the dashboard in Settings. Give devices names, obtain each peer’s pairing key on its authenticated Settings page, and add its hostname/IP/key to the dashboard. Create named groups.

Groups send individual commands and report each result. Accepted means received, not physical travel completed. An offline/faulted member can fail while others move. Starts are approximate, not synchronised. Each device continues its own schedules if the dashboard is offline. Reserve router addresses or keep peer addresses updated.
