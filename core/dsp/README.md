# DSP Engine

The DSP engine will be implemented as a deterministic real-time processing chain.

Requirements:

- No dynamic allocation on the real-time audio thread.
- Parameter changes are lock-free or applied at safe block boundaries.
- 32-bit floating point internal processing.
- Final limiter whenever gain can exceed unity.
- Bypass must reproduce the input within expected floating-point tolerance.

Planned components:

- biquad EQ
- bass shelf
- treble shelf
- loudness curve
- compressor
- limiter
- output gain
