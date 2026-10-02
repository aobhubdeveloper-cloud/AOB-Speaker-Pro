# AOB Speaker Pro

**Turn Your Android Into Your PC Speaker.**

AOB Speaker Pro is an open-source, low-latency audio streaming system for sending Windows PC audio to Android phones and tablets over local Wi-Fi or USB networking.

## Product goals

- Excellent perceived audio quality on Android devices
- Low and predictable latency
- Automatic discovery and reconnection
- Robust playback on unstable Wi-Fi
- Safe loudness enhancement with DSP and clipping protection
- Multiple Android receivers
- Windows 10/11 first-class support
- Open protocol and reproducible builds

## Planned architecture

```
Windows 10/11
  WASAPI Loopback
       |
       v
 Audio Engine -> DSP/EQ -> Opus -> Adaptive Transport
       |                              |
       +------------------------------+
                                      v
                           Wi-Fi / USB networking
                                      |
                                      v
                               Android Receiver
                                      |
                              Jitter + Drift Control
                                      |
                               Opus Decode / DSP
                                      |
                                  Oboe / AudioTrack
                                      |
                                      v
                                   Speaker
```

## Repository layout

```
apps/
  windows/                 Windows desktop application
  android/                 Android receiver application
core/
  audio/                   Audio capture/playback abstractions
  codec/                   Opus integration
  dsp/                     Audio enhancement pipeline
  network/                 Transport, discovery, telemetry
  protocol/                Wire protocol definitions
docs/
  architecture.md
  protocol.md
  audio-quality.md
  development.md
third_party/
  THIRD_PARTY_NOTICES.md
.github/
  workflows/
```

## Key features planned

- WASAPI loopback capture
- 48 kHz stereo pipeline
- Opus low-delay encoding
- Adaptive jitter buffer
- Clock/drift correction
- mDNS discovery
- Wi-Fi streaming
- USB networking
- Automatic reconnect
- Multi-device streaming
- 10-band EQ
- Bass boost
- Treble enhancement
- Loudness processing
- Compressor
- Limiter / anti-clipping
- AOB Auto mode
- Per-device volume
- Live latency/jitter/packet-loss telemetry
- Windows system tray controls
- Android foreground service and screen-off playback

## Design principles

1. Audio quality is preferred over unnecessary bandwidth savings.
2. Low latency must not come at the cost of unstable playback.
3. Loudness enhancement must include clipping protection.
4. Recovery from temporary network problems should be automatic.
5. Third-party code must be used only under compatible licenses with attribution.
6. The default experience should require minimal technical configuration.

## Reference projects

AOB Speaker Pro is independently implemented, while studying relevant open-source techniques from projects such as:

- Audio Share
- LAN Audio
- AudioStream
- other compatible open-source audio streaming implementations

See `third_party/THIRD_PARTY_NOTICES.md` for attribution policy.

## License

MIT License. See [LICENSE](LICENSE).

## Status

Early development — architecture and scaffold stage.

