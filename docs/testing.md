# Testing Strategy

## Unit tests

Test in isolation:
- packet encode/decode
- sequence arithmetic
- timestamp arithmetic
- jitter estimator
- adaptive buffer controller
- Opus configuration
- EQ coefficient generation
- compressor
- limiter
- gain staging

## Integration tests

### Windows capture
Feed a deterministic generated signal into a test render endpoint and verify capture output.

### Network
Use a deterministic packet simulator to inject:
- loss
- duplication
- reordering
- jitter
- bursts

### Android
Run decoder and jitter tests on host where possible; run playback tests on physical devices.

## Golden audio tests

Maintain short reference signals:
- impulse
- sine
- sweep
- speech sample
- music-like synthetic mix

Compare:
- RMS
- peak
- clipping
- frequency response
- expected latency behavior

Do not commit copyrighted commercial music into test fixtures.

## Device matrix

At minimum test:
- Android low-end
- Android mid-range
- Android flagship
- 44.1 kHz-native device
- 48 kHz-native device
- stereo speaker device
- wired headset
- USB audio output if relevant

## Manual test checklist

1. Start Windows app.
2. Start Android receiver.
3. Discover device.
4. Connect.
5. Play system audio.
6. Toggle each preset.
7. Enable/disable boost.
8. Change Android volume.
9. Lock phone.
10. Unlock phone.
11. Temporarily disable Wi-Fi.
12. Restore Wi-Fi.
13. Stop stream.
14. Reconnect.
15. Close both apps cleanly.
