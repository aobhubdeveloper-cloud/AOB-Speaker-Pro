# Development

## Initial toolchain

### Windows
- Visual Studio 2022 with C++ desktop tools
- CMake
- Rust toolchain
- Android Studio / SDK / NDK

### Android
- Kotlin
- Jetpack Compose
- Android NDK
- Oboe

## Engineering workflow

- Keep platform-specific code behind stable interfaces.
- Write unit tests for codec, packet serialization, jitter behavior, and DSP gain staging.
- Run static analysis and formatting in CI.
- Build Windows and Android artifacts in GitHub Actions.
- Keep third-party notices synchronized with dependency changes.

## First milestones

1. WASAPI capture proof of concept
2. Single-device Wi-Fi stream
3. Android playback
4. Adaptive jitter and drift correction
5. DSP engine
6. USB transport
7. Multi-device streaming
8. Packaging and release automation
