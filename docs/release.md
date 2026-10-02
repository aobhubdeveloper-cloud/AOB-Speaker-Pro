# Release Process

## Versioning

Use Semantic Versioning.

- MAJOR: incompatible protocol/API change
- MINOR: backwards-compatible feature
- PATCH: bug/security fix

## Release artifacts

Windows:
- portable ZIP
- installer
- SHA-256 checksums

Android:
- universal debug APK for testing
- release APK/AAB as appropriate
- SHA-256 checksums

## Reproducibility

Pin:
- compiler/toolchain versions
- dependency versions
- Android Gradle Plugin
- NDK
- Rust toolchain
- Opus version

## Release notes

Every release documents:
- new features
- audio changes
- latency changes
- compatibility changes
- security fixes
- known issues
- third-party dependency changes
