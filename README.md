# fabric-ingest-edge

The Ingest edge. It terminates the client transport, decodes player input datagrams, and hands the
result to the game data plane over iceoryx2.

```
ingest/                  this edge
transport/            QUIC and WebTransport
thirdparty/harness    fabric-harness: the iceoryx2 C ABI and the shared limits
thirdparty/picoquic   QUIC, HTTP/3 and WebTransport, with h3zero
thirdparty/mbedtls    TLS 1.3, with its framework, both as subtrees
```

## What an edge is

An edge obeys every plane rule and adds one capability, the network.

- **It holds no authority.** `Weft.Authority` decides which controller drives an avatar and
  `Weft.Zone.drive/4` enforces it, because two connections may land on two machines that
  never talk to each other.
- **It runs no simulation.** It has no tick.
- **It keeps no durable state.** Nothing here survives a restart, because nothing here is the
  truth about anything.

So an edge is the one place with a listening socket, and the one place with nothing worth
stealing.

## The packet path is C++

The transport decodes every datagram, so the language is a packet-rate decision. Measured on
one machine against a bar of 15 M snapshots per second per core:

| | rate |
| --- | --- |
| C++, `memcpy` decode | **841.51 M/s**, 56 times the bar |
| a scripting runtime | 5.70 M/s, 2.6 times under |

One crossing into a scripting runtime costs 117.8 ns and the whole per-packet budget is
66.7 ns. Policy that changes often lives in
[`fabric-janet-plane`](https://github.com/v-sekai-multiplayer-fabric/fabric-janet-plane),
which is a plane on the bus and never appears in this path.

## The wire

`XRGridEntityPacket`, 100 bytes, specified in `lean-entity-packet` and modelled in Lean with
a `packet_golden.csv` of canonical bytes. Anything written here passes those vectors rather
than asserting compatibility. **The packet is the schema and the compression is the
transport**, so fields stay wide and the stream is delta coded.

## TLS

mbedtls, through picotls, which is the backend the Godot client's `WebTransportPeer` uses. One
QUIC implementation and one TLS library on both ends, and
`thirdparty/picoquic-godot-patches` applies.

HTTP/3 and WebTransport come from picoquic's own `picohttp`: `h3zero_server.c`,
`h3zero_common.c` and `webtransport.c`. An edge serves no web pages, so it links no HTTP
server library.

## Build

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Every dependency is a subtree, so a clone builds with no submodule fetch and no network.
