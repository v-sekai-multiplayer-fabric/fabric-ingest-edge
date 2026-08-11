# The harness, and the proof that its bus works.
#
# This image exists to run the proof on real infrastructure, not to ship a plane. A plane
# builds its own image and takes this repository as a subtree.
#
# iceoryx2 builds in a stage of its own, so the Rust toolchain never reaches the runtime
# image. The harness links none of it: iceoryx2.sigs becomes a dlsym dispatch table, and
# the library arrives through dlopen at start.
#
#   fly deploy --config fly/fly.toml
ARG ICEORYX2_VERSION=0.9.3

FROM docker.io/library/rust:1.90-bookworm AS iceoryx
ARG ICEORYX2_VERSION
RUN apt-get update && apt-get install -y --no-install-recommends cmake ca-certificates curl \
  && rm -rf /var/lib/apt/lists/*
RUN curl -fsSL "https://github.com/eclipse-iceoryx/iceoryx2/archive/refs/tags/v${ICEORYX2_VERSION}.tar.gz" \
      | tar -xz -C /tmp \
  && cmake -S "/tmp/iceoryx2-${ICEORYX2_VERSION}" -B /tmp/ice-build \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/iceoryx -DBUILD_EXAMPLES=OFF \
  && cmake --build /tmp/ice-build -j \
  && cmake --install /tmp/ice-build \
  && rm -rf /tmp/ice-build "/tmp/iceoryx2-${ICEORYX2_VERSION}"

FROM docker.io/library/debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
      cmake make g++ python3 ca-certificates \
  && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . /src
# No iceoryx2 on the include or library path. That is the point: this builds on a machine
# that has never seen it, and fails at start rather than at link if it is absent.
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j

FROM docker.io/library/debian:bookworm-slim
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 \
  && rm -rf /var/lib/apt/lists/*
COPY --from=iceoryx /opt/iceoryx /opt/iceoryx
COPY --from=build /src/build/weft-harness-publisher /usr/local/bin/
COPY --from=build /src/build/weft-harness-subscriber /usr/local/bin/
COPY --from=build /src/build/weft-harness-bench_send /usr/local/bin/
ENV LD_LIBRARY_PATH=/opt/iceoryx/lib64
COPY fly/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
