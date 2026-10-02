# Audio Quality Strategy

## Internal format

The reference pipeline is 48 kHz stereo floating-point PCM.

## Enhancement

AOB Speaker Pro uses gain staging rather than raw volume multiplication.

Recommended order:

1. Input trim
2. Equalizer
3. Bass/treble enhancement
4. Loudness processing
5. Compressor
6. Output gain
7. True-peak style limiter

The implementation must provide a bypass path for users who want the original stream.

## Presets

- Original
- Balanced
- Music
- Movie
- Gaming
- Voice
- Bass Boost
- Loudness
- AOB Max
- Custom

## Quality modes

- Ultra Low Latency
- Balanced
- High Quality
- Adaptive

The UI should expose simple choices and keep advanced codec/buffer controls under an advanced panel.
