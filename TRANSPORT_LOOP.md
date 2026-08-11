# The event loop belongs to iceoryx2

An edge waits on the bus, because that is how it hands a decoded packet to a plane. If it also
waits on the network with something else, the process has two event loops. That is what h2o is
doing here today, and replacing h2o with `poll` would keep the shape and only change the name.

**So the loop is iceoryx2's, and the UDP socket attaches to it.** One wait, one wakeup, one
place that decides what happens next.

## What h2o is actually providing

Not HTTP/3. picoquic and `h3zero` do that. h2o appears in `transport/webtransport_server.c`
twenty-one times and supplies exactly this:

    h2o_loop_t, h2o_evloop_run, h2o_evloop_socket_create,
    h2o_socket_t, h2o_socket_close, h2o_socket_read_start, h2o_socket_read_stop

An event loop and a socket wrapper, over three descriptors: the UDP socket, picoquic's own
protocol timer as a `timerfd`, and the fixed-rate zone tick as a second `timerfd`. The file
already says it never calls `h2o_evloop_run` for the thread and drives picoquic itself, so what
is left is readiness notification and nothing else.

An edge that terminates a transport has no use for an HTTP server library, and this one was
never using it as one.

## Why this is not done yet

`thirdparty/harness/iceoryx2.sigs` binds 26 symbols: the node, the service, publisher,
subscriber and sample. **There is no WaitSet, no Listener, no event service and no way to
attach a file descriptor.** The only waiting primitive is

    int iox2_node_wait(iox2_node_h_ref, uint64_t sec, uint32_t nsec);

which is a periodic sleep. A periodic sleep cannot wake on a packet, so the harness cannot be
this loop until it binds more of the C API.

iceoryx2 has the pieces. The harness has not been asked for them.

## The order of work

1. **`fabric-harness` first.** Add the WaitSet and Listener symbols to `iceoryx2.sigs` and
   regenerate the dlsym table. The harness is a subtree here, so this lands upstream and is
   pulled in, and both edges get it at once.
2. **Attach the three descriptors** to the WaitSet: the UDP socket, and the two timerfds.
3. **Drive picoquic from those wakeups**, which is what the code already does, minus h2o.
4. **Then `cmake/picoquic.cmake` moves to mbedtls.** It cannot move earlier: h2o hard-requires
   OpenSSL, so OpenSSL is linked while h2o is here whatever picoquic uses. Dropping h2o is what
   makes one TLS library possible, and mbedtls is already vendored and building.

Step 4 is the reason the order matters. The TLS choice is downstream of the loop choice, and
they looked independent until h2o turned out to be the thing forcing both.
