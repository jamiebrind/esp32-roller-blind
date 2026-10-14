# Bill of materials — one blind

Current build: **12V external power, ESP32-C3, DRV8871, encoder, Mini360, no battery and no buttons**. Links are UK product or search links, not a live priced basket. Stock, seller and variant must be checked when ordering. Quantities for small consumables are approximate. [CSV version](BOM.csv).

| Item | Qty | Specification | UK link | Fit / verification notes |
|---|---|---|---|---|
| Motor | 1 | JGY-370, 12V, 20RPM, six-wire encoder, 6mm D shaft | [Link](https://www.amazon.co.uk/dp/B07DGBYJ63) | Builder’s selected Hilitand listing; choose the matching variant. Delivered label SKU-GS00127-02. Listings can change. |
| Controller | 1 | ESP32-C3 SuperMini, USB-C, 4MB flash | [Link](https://www.amazon.co.uk/dp/B0D2DH1F1H) | C3 SuperMini class board; check actual pin labels. Not C6/S3. |
| Motor driver | 1 | DRV8871 single DC motor driver breakout | [Link](https://www.amazon.co.uk/dp/B0DQCK7FLL) | Builder-selected Tiardey listing; IN1/IN2, POWER and MOTOR terminals. Check board identity and current-limit resistor. |
| Buck converter | 1 | Mini360 adjustable DC-DC step-down, 12V to 3.3V | [Link](https://www.amazon.co.uk/dp/B0DY14VBW6) | Builder-selected listing; verify Mini360-style board, polarity and stable loaded output before connecting ESP. |
| DC socket | 1 | DC-005-style female 5.5 × 2.1mm barrel socket | [Link](https://www.amazon.co.uk/s?k=DC-005+5.5+2.1+female+PCB+power+jack) | Search link. Match measured body 8.92 × 10.74 × 13.81mm; different sockets may not fit. |
| External supply | 1 | Regulated UK 12V DC centre-positive adapter, matching barrel plug | [Link](https://www.amazon.co.uk/s?k=12V+2A+UK+power+supply+5.5x2.1+centre+positive) | 2A-rated supply is a candidate; confirm actual motor startup/stall needs. Builder’s exact adapter is unrecorded. |
| Filament | As needed | SUNLU PLA+, 1.75mm, Bone White, 1kg spool | [Link](https://www.amazon.co.uk/dp/B0F42MT8DP) | Exact filament reported by builder; one spool is not consumed per unit. |
| Prints | 1 set | Housing, cover, electronics insert, bead wheel | [Link](../cad/stl/) | Four supplied V15 STL files; no buttons or retainers. |
| Motor fixings | As fitted | Short metric screws matching gearbox threads | [Link](https://www.amazon.co.uk/s?k=M3+machine+screw+assortment+4+6+8+10mm) | Candidate M3 kit only. Actual thread/count/length and safe insertion depth unconfirmed; do not force or bottom out. |
| Wheel retention | 1 set as required | Retainer/washer matched to the shaft and wheel | [Link](https://www.amazon.co.uk/s?k=6mm+shaft+retaining+washer) | Photo shows metal wheel retention; exact type/size unconfirmed. Inspect actual shaft before ordering. |
| Wall fixings | 2 | Nails or screws suitable for mounting surface, shaft <3.2mm | [Link](https://www.amazon.co.uk/s?k=small+round+head+nails+3mm) | 3.2mm holes, 76mm centres. Heads must retain ears; exact builder fixings and wall rating unconfirmed. |
| Signal wire | Small quantity | 26AWG stranded hookup wire | [Link](https://www.amazon.co.uk/s?k=26AWG+silicone+wire+kit) | Short signal leads; retain motor harness where practical. |
| Power wire | Small quantity | 22AWG stranded red/black hookup wire | [Link](https://www.amazon.co.uk/s?k=22AWG+silicone+wire+red+black) | For short power paths; route clear of moving wheel. |
| Insulation | Small quantity | Heat-shrink and insulating sheet/pads | [Link](https://www.amazon.co.uk/s?k=heat+shrink+tubing+electrical+insulating+sheet) | Insulate exposed joints and underside solder pads. |
| Strain relief | As needed | Small cable ties / internal tie mounts | [Link](https://www.amazon.co.uk/s?k=mini+cable+ties+adhesive+mounts) | Keep cable forces off solder joints and socket glue bond. |
| Jack adhesive | Small quantity | Adhesive compatible with printed PLA+ and socket plastic | [Link](https://www.amazon.co.uk/s?k=two+part+epoxy+plastic+adhesive) | Search/example category, not the builder’s confirmed glue. Test a spare printed coupon and allow full cure. |
| USB cable | 1 reusable | USB-C data cable for initial programming | [Link](https://www.amazon.co.uk/s?k=USB+C+data+cable) | Not a charge-only cable. Disconnect 12V before service USB. |

## Tools

3D printer, soldering iron and electronics solder, wire stripper/cutter, multimeter with continuity/DC voltage, suitable small drivers and a mounting tool appropriate to the surface. A current-limited bench supply is useful for first power-up. Tools and filament spool are shared purchases, not the per-unit cost.

## Measured motor and jack

- Gearbox: 46 × 32.1 × 22.3mm excluding shaft/bosses.
- Shaft: nominal Ø6mm, 5.4mm across the flat, 14mm exposed from the central boss.
- Motor can: Ø24.15mm, approximately 39mm long.
- Jack body: 8.92 × 10.74 × 13.81mm; cradle clearance envelope 9.22 × 11.04 × 14.11mm.

The gearbox mounting-hole spacing was retained from the previous motor model rather than remeasured. Compare your actual motor before printing. These dimensions describe the delivered unit, not every motor sold under the listing.

The build uses the encoder motor’s original six wires. No separate encoder board, external limit-switch board, battery-management board or USB-PD board is required by this version.
