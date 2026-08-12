# ─────────────────────────────────────────────────────
#  FluxDB Server — Multi-stage Dockerfile
#  Stage 1: Build C++ binary
#  Stage 2: Minimal runtime image
# ─────────────────────────────────────────────────────

# ── Stage 1: Builder ──────────────────────────────────
FROM ubuntu:22.04 AS builder

# Avoid interactive prompts during apt
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libssl-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source
COPY . .

# Build with CMake (FetchContent will download cpp-httplib)
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel $(nproc)

# ── Stage 2: Runtime ──────────────────────────────────
FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y libssl3 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy compiled binary from builder
COPY --from=builder /build/build/fluxdb /app/fluxdb

EXPOSE 6379 8080

ENTRYPOINT ["/app/fluxdb"]
CMD ["--tcp-port", "6379", "--api-port", "8080", "--max-keys", "10000"]
