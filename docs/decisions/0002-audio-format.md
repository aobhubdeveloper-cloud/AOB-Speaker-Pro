# ADR 0002 — Internal audio format

## Decision

Use 48 kHz stereo floating-point PCM as the reference internal representation.

## Rationale

48 kHz is broadly supported by Windows and Android audio paths and aligns well with common low-latency media pipelines.

## Consequence

Other source rates must be resampled at the capture boundary, and output devices may require an additional platform conversion.
