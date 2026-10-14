# Print and assemble

## 1. Check your blind and parts

Check the actual motor, shaft and socket against [BOM measurements](BOM.md). Match the wheel to your bead chain: the builder’s tested chain is not a universal standard. Confirm the chain connector can travel through the wheel path without jamming. Plan a secure mounting position and retain the blind’s appropriate chain safety/tension arrangement.

## 2. Print

Use the four [V15 STL files](../cad/stl/). The photographed build used:

| Setting | Builder-reported value |
|---|---|
| Printer | Bambu Lab P1S |
| Filament | SUNLU PLA+ 1.75mm, Bone White |
| Filament preset | Generic PLA |
| Process preset | 0.2mm Strength |

Exact nozzle size, temperatures, infill, wall count, support settings, plate type and preset version were not supplied. Use your installed profile as the starting point and inspect the slicer preview. The STLs are not a ready-to-print 3MF. Inspect support/bridging beneath the jack cradle and orient the wheel to preserve its bead pockets and shaft fit. Remove support and burrs without altering bead grip. PLA+ can soften in a hot window; the prototype is not a temperature-qualified product.

Dry-fit the wheel before assembling the case. It should fit the D-shaft without excessive force or splitting. The small bore-entry chamfer helps insertion, but does not compensate for inaccurate printing. Check the chain against a wheel test print before committing to installation.

## 3. Fit the DC jack

With the cover removed, dry-fit the measured socket into the open-top U cradle at the bottom end of the housing. Align the socket’s front face with the flat outer landing. Check the solder tabs clear the insert and cover. The nominal cradle is 9.22 × 11.04 × 14.11mm; the reference CAD socket is an envelope, not a manufacturer model.

Test your adhesive on spare printed material and socket plastic. Glue the outside of the socket to the cradle; keep glue out of the electrical contacts and plug bore. Hold it flush until the adhesive’s full cure time. Provide internal cable strain relief rather than relying on the glue joint to resist repeated pulling.

## 4. Install motor and boards

Secure the gearbox using fasteners that match its threads, with safe insertion depth. Exact screw size/count/length is not confirmed in this build record. Dry-fit the motor and electronics insert; check the cover closes without trapping cables. Insulate board undersides and exposed joints. Secure boards so they cannot rattle into the motor or socket. Do not cover the ESP antenna with metal or place loose wires in the chain path.

Follow [WIRING.md](WIRING.md), then install firmware and test encoder feedback before putting the wheel on the blind chain. The Mini360 must be adjusted and measured before powering the ESP.

## 5. Wheel and cover

Fit the wheel and appropriate shaft retention. The photos show a metal retainer/washer arrangement; its exact specification has not been confirmed. Do not copy it using guessed screw threads or load the shaft with excessive axial force. Check the wheel rotates freely, chain seats in the pockets, and the case/cover do not rub. There are no physical buttons in V15.

## 6. Mount

The external ears use two Ø3.2mm holes spaced 76mm apart. Select fasteners for the actual surface, with shafts that clear the holes and heads that retain the ears. Do not hammer through a motor/electronics enclosure or strike the printed ears. Use a gentle final seating method; the prototype has no tested mounting load rating. Secure the housing against twisting so chain tension cannot pull it sideways.

![Installed unit and exposed mounting ear](images/installed-side.jpg)

![Overall installation](images/installed-overview.jpg)

## 7. Commission

Run a motor-off-chain startup/reset test, verify direction and encoder counts, then attach the chain and calibrate while supervising. Test both limits repeatedly under load, Stop, power-loss recovery and a schedule a few minutes ahead. Do not enable unattended schedules until these tests pass. See [OPERATION.md](OPERATION.md).
