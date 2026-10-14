# Arduino controller 2.0.4

See [installation/build instructions](../docs/FIRMWARE.md), [wiring](../docs/WIRING.md) and [operation](../docs/OPERATION.md).

- ScheduledBlind/: sketch and core logic, custom partitions.csv, generated embedded assets.
- web/: editable webpage source.
- tools/embed_web.py: regenerate WebAssets.h after webpage edits.
- tools/release_manifest.py: create the matching OTA manifest from an application binary.
- tools/preview_server.py: local browser simulation only; demo password `demo-blind` is not a real-device password.
- build/: 2.0.4 initial USB images plus application/manifest for wireless updates.
- tests/: original portable test sources; no compiled host tools or private logs.

## Host tests

Use Python 3, Node.js and a C++17 compiler. C++ includes refer to the source tree; ArduinoJson 7.4.3 headers are needed by settings/JSON tests.

Example from firmware/tests (replace the header path with your installation):

```sh
c++ -std=c++17 -O0 -I /path/to/ArduinoJson/src test_scheduler.cpp -o test_scheduler
./test_scheduler
node --test test_web.mjs
python test_partition.py
python test_release.py
python test_preview_upload.py
```

Compile and run each test_*.cpp similarly. On Windows a compiler such as Zig can use `zig c++`, with a .exe output. The machine-specific original test runner is deliberately omitted. Python preview tests need the included preview server but never access the actual motor. Do not publish credentials or machine-specific configuration files when developing.
