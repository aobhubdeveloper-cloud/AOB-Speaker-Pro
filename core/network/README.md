# Network Engine

The network engine will separate:

- Discovery/control traffic
- Real-time audio traffic
- Telemetry

The audio plane should avoid TCP head-of-line blocking.

The initial transport target is UDP on the local network, with a later USB networking path using the same protocol.
