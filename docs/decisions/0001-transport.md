# ADR 0001 — Real-time audio transport

## Decision

Use UDP for the real-time audio plane and a reliable control channel for session setup and telemetry.

## Rationale

Audio playback is time-sensitive. A delayed packet can be less useful than a lost packet. The receiver can conceal isolated losses and maintain a bounded jitter buffer without forcing later audio to wait behind an earlier missing packet.

## Consequence

The protocol must explicitly handle sequence numbers, timestamps, reordering, packet loss and receiver health.
