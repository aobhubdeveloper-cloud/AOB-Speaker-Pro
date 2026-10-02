# Quality Gates

A release is not considered production-ready merely because it plays audio.

## Functional

- Windows system audio reaches Android.
- Android can play while screen is off.
- Automatic reconnect works after a short network interruption.
- Volume and mute work.
- DSP bypass works.
- Presets persist safely.
- Multiple source endpoints can be selected.

## Audio

- No persistent clicks/pops under normal network conditions.
- No unbounded queue growth.
- No clipping when limiter is enabled.
- Silence does not produce runaway CPU or bandwidth.
- Sample-rate conversion is deterministic.
- DSP parameters can change without audio-thread allocation.

## Latency

Measure end-to-end latency with an external synchronized test method rather than claiming a fixed number for every device.

Track:
- p50
- p95
- p99
- underrun count

Target classes:
- Ultra Low Latency: aggressive buffer target
- Balanced: stability/latency compromise
- High Quality: larger safety margin

Device and network conditions must be recorded with every benchmark.

## Reliability

Test:
- 0%, 1%, 3%, 5%, 10% simulated packet loss
- burst loss
- variable RTT
- Wi-Fi roaming
- temporary disconnect
- Android screen off/on
- Windows sleep/wake
- router restart
- app backgrounding
- phone battery saver

## Performance

Measure:
- Windows CPU
- Android CPU
- Android battery drain
- memory
- packet rate
- encoded bitrate
- audio callback overruns/underruns

## Security

- malformed packet rejection
- oversized packet rejection
- invalid sequence handling
- protocol version rejection
- pairing authorization
- no public-network assumptions

## Release gate

A release candidate requires:
- automated tests passing
- Windows release build passing
- Android release build passing
- protocol compatibility test passing
- representative device matrix passing
- no known P0/P1 audio corruption issues
