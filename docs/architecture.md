# Architecture

## Objectives

AOB Speaker Pro separates the application into capture, processing, encoding, transport, receiving, synchronization, and playback layers.

## Windows pipeline

1. WASAPI loopback captures the selected render endpoint.
2. Audio is normalized into the internal PCM representation.
3. DSP runs before encoding.
4. The Opus encoder produces fixed-duration real-time frames.
5. Transport adds sequence numbers, timestamps, stream identity, and telemetry.
6. Receiver health determines bitrate/buffer adaptation.

## Android pipeline

1. Receiver discovers or pairs with a Windows sender.
2. Packets are reordered and placed in an adaptive jitter buffer.
3. Lost packets are concealed where possible.
4. Clock drift is measured against stream timestamps.
5. Opus frames are decoded to PCM.
6. Android playback uses Oboe where available, with a compatible AudioTrack fallback.
7. Android DSP/volume/limiter runs before final playback.

## Reliability

The transport must tolerate burst loss and variable Wi-Fi latency without creating long TCP-style head-of-line blocking. The control plane is separate from the real-time audio plane.

## Multi-device synchronization

Each stream carries a sender clock/timestamp reference. Receivers estimate relative clock offset and rate. Synchronization is applied gradually to avoid audible pitch modulation or abrupt buffer jumps.

## Audio safety

The DSP chain must keep samples inside the configured output headroom. A final limiter is mandatory whenever the user enables gain above unity.
