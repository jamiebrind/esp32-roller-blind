# Firmware release preference

The owner requires **every new firmware release to be published to GitHub as a versioned release**. Do not finish a firmware-release task with only an untagged source push or a local ZIP.

For each release:

1. Use a new semantic version and keep the sketch version, manifest and release notes consistent.
2. Build and verify the firmware, run the relevant regression gates and check that the application matches its manifest length and SHA-256.
3. Update CHANGELOG.md and any affected build/operation documentation.
4. Commit and push the source and release artifacts to main.
5. Create an annotated `vMAJOR.MINOR.PATCH` tag at the release commit and push it.
6. Publish a GitHub Release for that tag, with the application `.bin`, matching `manifest.json`, and an update ZIP. Explain initial USB installation separately from application-only wireless updates.
7. Verify the public release and uploaded asset hashes. Exclude credentials, private device logs, local build options and generated compiler/debug files.

An existing released version/tag must not be silently overwritten. Use a new version for subsequent firmware changes. See docs/RELEASING.md.
