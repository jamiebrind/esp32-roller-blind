# V15 CAD and print files

[View the six-image Fusion CAD gallery](../docs/CAD_GALLERY.md).

- [Fusion native F3D](RollerBlind_V15.f3d)
- [STEP assembly](RollerBlind_V15.step)
- [Four STL print parts](stl/)

All dimensions are millimetres. The native file includes reference components for fit; do not print the electronics/motor/jack references. Only print the four named STLs. A hidden duplicate gear can remain in the native document; it is not an extra required part.

| File | Purpose | Bounding size, mm |
|---|---|---|
| 01_Original_housing_V15.stl | Housing, jack cradle and external ears | 90 × 140 × 32 |
| 02_Original_cover_V15.stl | Blank front cover, no buttons | 50 × 140 × 6.4 |
| 03_Tested_original_0p5_working_wheel_V15.stl | Tested bead wheel | 35 × 35 × 13 |
| 04_Original_electronics_insert_V15.stl | Internal board carrier | 45.2 × 69.849 × 24 |

Wheel bore: 6.30mm round diameter, 5.60mm across flat, 0.5mm entry chamfer. These are CAD dimensions; printed dimensions depend on extrusion and calibration. Preserve the bead-pocket geometry when adjusting the shaft fit. “0p5” is the historical test revision name, not a measured universal bead spacing.

External ears: 5mm thick, Ø3.2mm holes, 76mm centre spacing, back face on the same plane as the case. Footprint including ears: 90 × 140mm. Assembled case depth including cover: 33.4mm, excluding wheel.

![CAD assembly](../docs/images/V15_assembled.png)

![External mounting ears](../docs/images/V15_top_mounts.png)

![Inside jack cradle](../docs/images/V15_jack_cradle.png)

The native timeline is healthy and the print bodies were verified as connected solids. The measured socket envelope clears housing/cover/insert. A tiny original mesh-derived housing/insert nominal contact (~0.037mm³) remains; no new interference was introduced. Actual glue joint, chain fit, print clearance and mounting strength require physical tests.
