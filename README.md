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

## TLS is mbedtls, and h2o is dropped

**picoquic with h3zero, and mbedtls.** No h2o, and no OpenSSL.

`cmake/picoquic.cmake` still records the older decision, which was OpenSSL. That decision was
correct when it was made and its reason has since gone away, so it is worth writing down why
rather than silently flipping it.

The reason was h2o. `libh2o-evloop` hard-requires OpenSSL in its own CMakeLists, with no
option to disable, so OpenSSL was linked into this binary whatever picoquic used. Carrying
mbedtls as well would have meant two TLS libraries instead of one, and OpenSSL was the one
that could not be removed.

**h2o can be removed.** It arrived here because the transport moved out of `zone-server-h2o`
verbatim, and `transport/webtransport_server.h` still includes `<h2o.h>` for that reason and
no other. picoquic already ships HTTP/3 and WebTransport of its own, in
`thirdparty/picoquic/picohttp`: `h3zero_server.c`, `h3zero_common.c` and `webtransport.c`,
documented in `thirdparty/picoquic/doc/pico_webtransport.md`. An edge terminates a transport
and hands the result to a plane. It serves no web pages, so an HTTP server library is a
dependency it never needed.

Once h2o goes, nothing forces OpenSSL, and mbedtls is the better answer:

- **One TLS library, not two.** That was the original goal, and it is now reachable.
- **The same backend the client uses.** The Godot fork's `WebTransportPeer` is picoquic with
  mbedtls, so server and client share one QUIC implementation and one TLS backend rather than
  meeting only at the wire.
- **The patches apply again.** `thirdparty/picoquic-godot-patches/0002-godot-fixes.patch`
  carries mbedtls-specific glue, buffer-based private key loading, ECDSA raw-to-DER signature
  conversion, and packet-loop atomics. Under OpenSSL that work is carried and unused.
- **A smaller image**, which is a cost on Fly.

picoquic tests this backend in its own CI, at `.github/workflows/mbedtls.yml`.

### What this needs

`transport/webtransport_server.h` includes `<h2o.h>` and has to stop. The WebTransport
session handling moves onto `h3zero_common` and `webtransport.c`, and `cmake/picoquic.cmake`
builds picotls against mbedtls rather than OpenSSL.
