# Milestones 1–6 implementation map
1. **First sound:** WASAPI shared-mode loopback → 48 kHz/2ch Opus → bounded UDP packets → Android native Opus decode → Oboe low-latency output.
2. **Enhancement:** shared float DSP engine with named presets, limiter and per-receiver settings contract.
3. **Connectivity:** UDP transport boundary, Android NSD discovery, explicit receiver port, bounded queues and service lifecycle for reconnect work.
4. **Multi-device:** packet timestamps/sequence IDs plus clock synchronization structures; each receiver can own independent DSP/volume while sharing a master stream clock.
5. **Productization:** Android Compose shell + foreground service; Windows sender executable boundary; shared core isolated from UI.
6. **Release/security:** CMake/Gradle dependency pinning, CI/test entry points, security policy, packet size/checksum validation and bounded receiver memory.
Production hardening still requires real-device latency measurements, automated reconnect tests, fuzzing, signed release artifacts and installer/Play packaging before a 1.0 release.
