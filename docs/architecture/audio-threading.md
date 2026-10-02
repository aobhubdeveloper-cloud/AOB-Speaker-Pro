# Real-time Audio Threading

The real-time callback path is treated as a hard real-time-ish path.

## Rules

Never perform these operations in the audio callback:
- network I/O
- file I/O
- blocking locks
- memory allocation
- logging with unbounded work
- expensive UI operations

Use lock-free or bounded queues between:
- capture and encoder
- decoder and jitter buffer
- jitter buffer and playback

This is especially important on Android, where Oboe's low-latency callback is expected to remain short and avoid bursty CPU work. Android guidance recommends high-priority callbacks and non-blocking communication for low-latency audio. citeturn0search0turn0search3
