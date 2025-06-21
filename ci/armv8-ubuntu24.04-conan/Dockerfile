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

RUN git clone https://github.com/conan-io/conan.git conan_src \
&& cd conan_src \
&& python3 -m pip install -e . --break-system-packages && conan profile detect --force

RUN gcc --version && conan --version

WORKDIR /workspace

COPY ./conanfile.py /workspace
RUN conan install . \
  --output-folder=build-arm \
  --build=missing \
  -c tools.system.package_manager:mode=install && \
  rm -rf build-arm
