# Security

AOB Speaker Pro is designed primarily for trusted local networks.

## Principles

- Do not expose the control service to the public Internet.
- Authenticate pairing before enabling persistent control.
- Treat received metadata as untrusted input.
- Bound packet sizes before parsing.
- Reject unsupported protocol versions.
- Never execute received data as code.
- Keep discovery local to the LAN.

Security-sensitive issues should be reported privately to the repository maintainers before public disclosure.
