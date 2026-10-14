# Firmware release process

Every firmware release is committed, tagged and published on GitHub.

1. Increment `FIRMWARE_VERSION` in `firmware/ScheduledBlind/ScheduledBlind.ino`.
2. Build with the pinned toolchain described in [FIRMWARE.md](FIRMWARE.md), then regenerate the matching application manifest.
3. Run the regression checks and review the firmware change. Record verification limits accurately.
4. Update [CHANGELOG.md](../CHANGELOG.md), documentation and compiled release files. Commit and push to `main`.
5. Create and push an annotated tag, for example:

```sh
git tag -a v2.0.4 -m "Firmware 2.0.4"
git push origin v2.0.4
```

6. Publish a GitHub Release for the tag. Attach:

   - `ScheduledBlind.ino.bin`: application-only image for the Firmware page.
   - `manifest.json`: its matching hardware/version/length/SHA-256 manifest.
   - An update ZIP containing both files and installation/release notes.

7. Verify that GitHub shows the intended tag/commit and all assets, and that downloaded assets match the local hashes. GitHub also supplies source ZIP/tar archives for the tagged commit.

Do not reuse an existing release version for changed firmware. Do not upload a merged image as an application-only update. Credentials and private device logs are excluded from both source and release assets.

Publishing a release makes it available for installation; it does not imply that every controller has already installed it. Release notes identify what has actually been tested.
