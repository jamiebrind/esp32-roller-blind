# Troubleshooting

## Webpage unavailable

- Wait at least 60 seconds after startup; retries occur every 30 seconds.
- Use 2.4GHz Wi-Fi. Check the actual IP in the router; DHCP may assign a new one.
- Try the announced unique `.local` hostname. A failed mDNS lookup alone does not mean the ESP is disconnected.
- With 12V off, use USB and STATUS to obtain the current IP. Historical logs are not retained across power loss by firmware 2.0.2.
- If USB works but 12V does not, check Mini360 output at ESP3V3/GND under load, wiring/ground continuity and motor interference. An LED does not establish stable voltage during Wi-Fi transmission. A multimeter may miss brief dips.
- Never connect USB while the Mini360 feeds ESP3V3. Do not raise the buck voltage to hide a bad connection.

## Motor turns, raw encoder count does not

Check 3.3V directly between encoder blue and black under the intended run supply. Check yellow continuity to GPIO3, green to GPIO4, and common black/ESP ground. The builder traced a missing-count issue to wiring and reported it working after correction. Encoder power was also lower under USB in one measurement; USB-only motor tests are not equivalent to the 12V run circuit.

To test ESP inputs without the encoder:

1. Power off/disconnect 12V and USB. Disconnect encoder yellow/green from GPIO3/4.
2. Power ESP with USB only; leave 12V off and observe raw count on the webpage.
3. Start with both inputs open (internal pull-ups make them HIGH).
4. Ground GPIO3; then also ground GPIO4; release GPIO3; then release GPIO4.

One ideal full quadrature cycle gives four counts in one direction; reverse the sequence for the opposite direction. Jumper bounce can add counts. Shorting GPIO3 and GPIO4 together is not a valid quadrature cycle. Use GND only, never 12V. Shut power off before reconnecting the encoder wires.

If this changes counts but the powered encoder does not, inspect its wiring/power and signal pulses at the motor. Swapping yellow/green reverses direction rather than stopping all counts. A stationary count is normal when the motor is stopped. Raw count starts at zero on reboot; a saved logical position value can still appear separately.

## Faults and calibration

“Encoder progress too slow” means feedback did not satisfy the configured progress threshold during commanded travel; it does not uniquely diagnose the cause. Check whether the motor physically moved, whether counts changed and whether the blind/chain is jammed. Do not simply disable the safeguard. A fault during calibration invalidates that calibration.

## Startup movement

Current firmware uses driver IN1 GPIO0 and IN2 GPIO1. GPIO7/6 in historical builds caused startup movement on this assembly. With motor off-chain, verify cold power-on and RESET remain stopped before mounting. If there is unexpected motion, remove power and check the board/wiring before enabling schedules.

## Schedule shows tomorrow

Check 24-hour time, selected weekdays, valid NTP time and whether today’s event was already recorded/skipped. The firmware does not replay passed events. The documented 02:45/14:45 confusion was a time-entry issue, not a next-day restriction.
