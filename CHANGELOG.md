# Changelog

## 2.0.3 — 14 October 2026

- Fix edited schedules being suppressed for the rest of the day after the same rule ID was executed or skipped.
- Record claims against the schedule definition, including relevant solar location and offset.
- Persist migration of 2.0.2 history before schedule edits are accepted; upgrading does not replay old events.
- Keep live settings identical to their stored JSON representation, making solar claim identities stable over restart.
- Unchanged rules retain duplicate-run protection; past times are never replayed on save.
- No motor wiring, calibration method, 60-second cap or reboot position-confirmation change.

Verified: 20 C++ suites, 10 JavaScript cases and 4 Python cases; independent review passed. Real Wi-Fi installation, reboot and retained settings/calibration/password were verified. No motor movement was commanded during installation; a physical scheduled-run test still requires the builder to confirm position and supervise.
