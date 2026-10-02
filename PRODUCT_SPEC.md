# AOB Speaker Pro — Product Specification

## Product promise

AOB Speaker Pro turns an Android phone/tablet into a high-quality speaker for Windows PC audio over a trusted local connection.

## Primary user journey

1. Install AOB Speaker Pro on Windows.
2. Install AOB Speaker Pro on Android.
3. Put both devices on the same Wi-Fi network, or connect by USB.
4. Start the Android receiver.
5. Windows discovers the device automatically.
6. Select a device and press Start.
7. PC audio plays through Android.
8. AOB Auto continuously balances latency, quality and stability.

## Supported connection modes

### Wi-Fi
Primary mode. Prefer 5 GHz when available.

### USB
Use Android USB networking/ADB transport where supported. USB must share the same media protocol as Wi-Fi so the audio engine remains transport-independent.

### Bluetooth
Optional future compatibility path. It is not the primary transport because platform Bluetooth audio paths can impose device-dependent latency and limitations.

## Audio quality

Reference stream:
- 48 kHz
- Stereo
- 32-bit float internally
- Opus for normal streaming
- PCM fallback for compatible high-bandwidth local links

User-facing quality:
- Ultra Low Latency
- Balanced
- High Quality
- Adaptive

## Enhancement engine

All enhancement is optional and bypassable.

- Preamp
- 10-band parametric EQ
- Bass boost
- Treble enhancement
- Loudness
- Compressor
- Limiter
- Output gain
- Presets
- Per-device volume

The limiter is always placed after gain-producing effects when those effects can exceed unity.

## AOB Auto

AOB Auto observes:
- packet loss
- jitter
- RTT
- receiver underruns
- buffer occupancy
- device capability
- selected quality mode

It can adjust:
- Opus bitrate
- packet/frame duration where safe
- jitter target
- recovery behavior

Adaptation must be gradual to avoid audible pumping.

## Multi-device

Phase 1 supports one receiver.
Later versions support several receivers from one Windows source and synchronize playback using media timestamps and clock-rate estimation.

## Privacy

No cloud account is required for local streaming.
Audio stays on the local connection by default.
Telemetry is local unless the user explicitly enables diagnostics export.

## Accessibility

- Keyboard navigable Windows UI
- Large touch targets on Android
- Clear connection states
- Screen-reader labels
- No critical status conveyed by color alone
