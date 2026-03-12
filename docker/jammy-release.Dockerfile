FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_DEFAULT_TRIPLET=x64-linux

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        curl \
        git \
        libseccomp-dev \
        ninja-build \
        pkg-config \
        python3 \
        tar \
        unzip \
        zip \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics \
    && "$VCPKG_ROOT/vcpkg" install \
        fmt \
        gtest \
        nlohmann-json \
        spdlog \
        stduuid

WORKDIR /workspace
COPY . .

RUN cmake --preset linux-release \
    && cmake --build out/build/linux-release --parallel \
    && ctest --test-dir out/build/linux-release --output-on-failure

FROM ubuntu:22.04 AS artifacts

COPY --from=builder /workspace/out/build/linux-release/SandboxRunner/SandboxRunner /artifacts/SandboxRunner
COPY --from=builder /workspace/out/build/linux-release/SandboxRunnerCore/libsandbox.so /artifacts/libsandbox.so
COPY --from=builder /workspace/policies/CXX_PROGRAM.json /artifacts/CXX_PROGRAM.json
