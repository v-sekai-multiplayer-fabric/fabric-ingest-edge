#!/bin/sh
# Runs both ends of the proof in one machine, and exits with the subscriber's status.
#
# One machine, and not two. iceoryx2 is shared memory: /dev/shm for the segments and
# /tmp/iceoryx2 for the service registry. Two Fly machines share neither, so a publisher
# in one and a subscriber in the other would find no service at all. That is the whole
# reason planes that talk over the bus ship together.
set -eu

COUNT=${WEFT_MESSAGES:-8}

weft-harness-subscriber "$COUNT" &
sub=$!
sleep 1
weft-harness-publisher "$COUNT"
wait "$sub"
