# fabric-ingest-edge

The ingest edge. An edge is a plane with networking, and this one terminates the client
transport and hands the decoded result to a plane over iceoryx2.

Split out of `fabric-edge`, which held both edges and is now archived. An edge is its own
process, its own repository, and its own container, so two edges are two repositories.

```
ingest/     this edge
transport/    QUIC and WebTransport, shared with the other edge by copy
thirdparty/harness    fabric-harness, the iceoryx2 C ABI and the shared limits
thirdparty/picoquic   vendored, the same sources the Godot client's backend uses
```

## What an edge is

It obeys every plane rule and adds exactly one capability, the network.

- **It holds no authority.** `Weft.Authority` decides which controller drives an avatar, in
  FoundationDB, because two connections may land on two machines that never talk.
- **It runs no simulation.** It has no tick of its own.
- **It keeps no durable state.** Nothing here survives a restart, because nothing here is the
  truth about anything.

So an edge is the one place with a listening socket and the one place with nothing worth
stealing.

## The packet path is C++, measured

The transport decodes every datagram, so the language is a packet-rate decision. Measured on
one machine, decoding a 12-byte entity update against a 15 M/s bar:

| | rate |
| --- | --- |
| C++, `memcpy` decode | **841.51 M/s**, 56 times the bar |
| Janet, byte assembly | 5.70 M/s, 2.6 times under |

One crossing into a scripting runtime costs 117.8 ns and the whole per-packet budget is
66.7 ns. So policy that changes often lives in `fabric-janet-plane`, which is a plane on the
bus and never appears in this path.

## TLS

OpenSSL, not mbedtls. `cmake/picoquic.cmake` records why: h2o's own CMakeLists hard-requires
OpenSSL, so OpenSSL was always linked here, and using mbedtls would mean carrying a second
TLS library rather than removing one. TLS 1.3 is a wire protocol, so a server on picotls's
OpenSSL backend still interoperates with a client on mbedtls.
