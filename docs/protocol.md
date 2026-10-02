# AOB Speaker Pro Protocol

The protocol is intentionally versioned and independently documented.

## Transport

- Audio plane: UDP
- Control plane: TCP or an equivalent reliable local-network channel
- Discovery: mDNS/DNS-SD on the local network
- Default media frame duration: 20 ms
- Reference sample rate: 48,000 Hz
- Reference channels: stereo

## Packet identity

Every audio packet contains:

- protocol version
- stream ID
- sender device ID
- sequence number
- media timestamp
- payload type
- frame duration
- Opus payload length
- optional integrity/check information

## Receiver behavior

The receiver tracks:

- packet loss
- reordering
- arrival jitter
- buffer depth
- estimated clock drift
- underruns

The receiver can report health data to the sender so the sender can adjust bitrate or request a higher/lower quality profile.

## Compatibility

Protocol changes that are not backward compatible increment the major protocol version.
