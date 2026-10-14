# Changelog

## 2.0.3 — prepared 14 October 2026

- Fix edited schedules being suppressed for the rest of the day after the same rule ID was executed or skipped.
- Record claims against the schedule definition, including relevant solar location and offset.
- Persist migration of 2.0.2 history before schedule edits are accepted; upgrading does not replay old events.
- Keep live settings identical to their stored JSON representation, making solar claim identities stable over restart.
- Unchanged rules retain duplicate-run protection; past times are never replayed on save.
- No motor wiring, calibration method, 60-second cap or reboot position-confirmation change.

The candidate has not been installed on the physical controller. Its current version remains 2.0.2 until authorised installation.
