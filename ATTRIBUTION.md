# Attribution and provenance

- Original author: **nik-po**, project **nik.po-roller-blind**.
- Electronics/firmware reference: https://github.com/nik-po/nik.po-roller-blind
- Original repository licence: https://github.com/nik-po/nik.po-roller-blind/blob/main/LICENSE.md — CC BY-NC-SA 4.0.
- Author-linked mechanical model: https://makerworld.com/en/models/3292116-smart-roller-blind-motor-esp32-home-assistant
- This adaptation and installation photographs: **jamiebrind**, 2026.

The original enclosure meshes were supplied by the builder from the author's mechanical project. They were used as the basis of the V15 model. The model is an adaptation rather than an independently designed enclosure. The native CAD may contain mesh-derived faces; it is not a fully recreated parametric history of the original author’s design.

Changes: measured JGY-370 motor clearance, tested alternative bead wheel with revised D-shaft bore, entry chamfer, button/retainer removal, filled button openings, internal jack cradle and flat flush landing, insert relief, and reinforced external mounting ears. The firmware here is a custom Arduino implementation using encoder position rather than the original project's timed ESPHome controller. It does not provide the original Home Assistant integration.

The original Git repository explicitly permits noncommercial adaptations under CC BY-NC-SA 4.0. The MakerWorld page's current detailed terms were not independently retrievable during preparation; refer to the original page for any notices applying to the source print files. This repository does not grant rights broader than those held in the source material. If its mechanical terms require a correction, please open an issue before redistributing the CAD further.

Arduino-ESP32, ESP-IDF, ArduinoJson and other dependencies are installed separately and retain their respective licences. Prebuilt firmware incorporates dependencies; upstream licence/notice references are listed in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
