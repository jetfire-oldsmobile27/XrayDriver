FROM arm64v8/ubuntu:24.04

RUN sed -i 's|http://archive.ubuntu.com/ubuntu/|http://mirror.yandex.ru/ubuntu/|g' /etc/apt/sources.list
RUN apt update 
RUN apt install -yq --no-install-recommends \
    build-essential \
    gcc \
    g++ \
    python3 \
    python3-pip \
    git \
    curl \
    pkg-config \
    ca-certificates \
    meson \
    ninja-build \
    cmake \
    && apt clean && rm -rf /var/lib/apt/lists/*

RUN pip3 install conan && conan profile detect --force

RUN gcc --version && conan --version

WORKDIR /workspace

COPY ./conanfile.py /workspace
RUN conan install . \
  --output-folder=build-arm \
  --build=missing 
