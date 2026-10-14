# Firmware installation and development

Current version: **2.0.3**, ESP32-C3, 4MB flash, DRV8871 GPIO0/1, encoder GPIO3/4. There is no default administrator password or embedded Wi-Fi password. Set your own credentials. Existing controllers keep compatible saved settings unless you erase flash.

## Initial install: prebuilt firmware

1. Disconnect 12V. Connect the ESP using a USB data cable.
2. Identify its serial port. If needed, hold BOOT, tap RESET, release BOOT to enter download mode.
3. Install Python and Espressif esptool: `python -m pip install esptool`.
4. From the repository root, run the following, replacing COM5 with your port (Linux/macOS: use its device path):

```sh
python -m esptool --chip esp32c3 --port COM5 --baud 115200 write-flash 0x0 firmware/build/ScheduledBlind.ino.bootloader.bin 0x8000 firmware/build/ScheduledBlind.ino.partitions.bin 0xe000 firmware/build/boot_app0.bin 0x10000 firmware/build/ScheduledBlind.ino.bin
```

These images use the supplied dual-slot partition table. Do not mix partition/bootloader files from a different build. Do not use erase-flash on a controller whose settings you want to retain. Reset after flashing if it remains in download mode.

## Set Wi-Fi and administrator password

Open a serial terminal at **115200 baud**, with newline line endings. Commands use **real TAB characters**, not the literal text `<TAB>`. Choose your own strong password:

```text
WIFI<TAB>YOUR_2.4GHZ_NETWORK<TAB>YOUR_WIFI_PASSWORD
ADMIN<TAB>YOUR_UNIQUE_ADMIN_PASSWORD
STATUS
```

Wi-Fi password: 8–63 characters. Admin password: 8–63 characters. Credentials are saved on-device; Wi-Fi saves after a successful connection. The serial setup does not echo them. Use a trusted serial terminal and do not publish its history.

Wait for connection; a failed attempt is retried every 30 seconds. STATUS prints the IP. The device announces its unique `blind-<device-id>.local` hostname, but mDNS support varies by client/network. The Wi-Fi network must be 2.4GHz. Reserve an address in your router if desired. This build configures the UK radio region (channel 13 permitted).

Disconnect USB before applying 12V. Open the device’s IP in a browser on the same LAN, log in and follow [calibration](OPERATION.md). Use local trusted Wi-Fi; HTTP credentials and pairing keys are not encrypted. Do not expose the control page directly to the Internet.

## Subsequent wireless updates

On the Firmware page select:

- `firmware/build/ScheduledBlind.ino.bin`
- Matching `firmware/build/manifest.json`

Do not select the bootloader, partitions, boot_app0 or a merged image. The updater stops the motor, pauses schedules, checks hardware/schema/file length/SHA-256 and writes the inactive application slot. Compatible settings remain. After reboot, confirm position before enabling automation. Interrupted transfer keeps the old image; a successfully transferred but faulty application can still require USB recovery. There is no promised application health-based rollback.

## Build from source

Install [Arduino CLI](https://arduino.github.io/arduino-cli/) and use ESP32 core **3.3.12**, ArduinoJson **7.4.3**, USB CDC enabled:

```sh
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli lib install ArduinoJson@7.4.3
python firmware/tools/embed_web.py
arduino-cli compile --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc,UploadSpeed=115200 --build-property upload.maximum_size=1966080 --output-dir firmware/build firmware/ScheduledBlind
python firmware/tools/release_manifest.py --binary firmware/build/ScheduledBlind.ino.bin --version 2.0.3 --output firmware/build/manifest.json
```

The sketch includes `partitions.csv`; each app slot is 1,966,080 bytes. Update the source FIRMWARE_VERSION and manifest version together for future releases. Rebuilding can change binary hashes; always regenerate the matching manifest. Source/web editing does not modify the installed device until you upload.

Arduino CLI can also install the compiled images over USB:

```sh
arduino-cli upload --fqbn esp32:esp32:esp32c3:CDCOnBoot=cdc,UploadSpeed=115200 --port COM5 --input-dir firmware/build firmware/ScheduledBlind
```

## Tests and evidence

The 2.0.3 release passed 20 C++ test executables, 10 JavaScript cases and 4 Python cases; compile used 1,303,375 bytes flash and 42,232 bytes static RAM. A real Wi-Fi update/reboot retained credentials and calibration. The builder subsequently reported working motor/encoder feedback and supplied installed photographs. Precise loaded stopping/force limits and a real multi-blind deployment are not established by those checks.

The `firmware/tests/` sources are included. JavaScript tests run with `node --test firmware/tests/test_web.mjs`; Python tests use Python 3. C++ tests require a C++17 compiler (some also require separately installed ArduinoJson headers). See [firmware/README.md](../firmware/README.md) for test commands. Automated browser file-picker upload was not verified; the device updater itself was tested via authenticated HTTP.

### Live schedule edit test (2.0.3)

On the installed prototype after calibration, a temporary evening rule raised to an encoder-reported 20%. Editing that same already-used rule to a later time then lowered to 0%. Both events ran, encoder counts changed, and the motor stopped with no fault. Original rules were restored. This checks the reported same-day edit defect; it is not a long-term stopping/obstruction qualification.
