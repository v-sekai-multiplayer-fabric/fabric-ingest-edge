# transport-ingest

The ingest transport layer. It terminates the client transport, decodes player input datagrams, and hands the result to the game interactor over iceoryx2.

It vendors its dependencies as subtrees rather than submodules: picoquic for QUIC, HTTP/3 and WebTransport, mbedtls for TLS 1.3, and `contract-bus` for the iceoryx2 C ABI and the shared limits. What is under `thirdparty/` is what `ls` says is under `thirdparty/`.

## What a transport layer is

It obeys every interactor rule and adds one capability, the network.

- **It holds no authority.** `Weft.Authority` decides which controller drives an avatar and `Weft.Zone.drive/4` enforces it, because two connections may land on two machines that never talk to each other.
- **It runs no simulation.** It has no tick.
- **It keeps no durable state.** Nothing here survives a restart, because nothing here is the truth about anything.

So it is the one place with a listening socket, and the one place with nothing worth stealing.

## The wire, and TLS

The packet path is C++, because the transport decodes every datagram and that makes the language a packet-rate decision. `TRANSPORT.md` has the measurement.

`XRGridEntityPacket`, 100 bytes, specified in `contract-entity-packet` and modelled in Lean with a `packet_golden.csv` of canonical bytes. Anything written here passes those vectors rather than asserting compatibility. **The packet is the schema and the compression is the transport**, so fields stay wide and the stream is delta coded.

TLS is mbedtls through picotls, the backend the Godot client's `WebTransportPeer` uses, so there is one QUIC implementation and one TLS library on both ends; `thirdparty/picoquic-godot-patches` applies. HTTP/3 and WebTransport come from picoquic's own `picohttp` — `h3zero_server.c`, `h3zero_common.c`, `webtransport.c`. This serves no web pages, so it links no HTTP server library.

## Build

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Every dependency is a subtree, so a clone builds with no submodule fetch and no network. `TRANSPORT.md` and `TRANSPORT_LOOP.md` carry the transport detail.
