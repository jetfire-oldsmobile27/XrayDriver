FROM arm64v8/ubuntu:22.04

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
    libatomic1 \
    clang \
    && apt clean && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/conan-io/conan.git conan_src \
&& cd conan_src \
&& python3 -m pip install -e .

# Создаем профиль Conan с использованием Clang
RUN mkdir -p /root/.conan2/profiles \
    && cat <<EOF > /root/.conan2/profiles/default
[settings]
arch=armv8
build_type=Release
compiler=clang
compiler.version=14
compiler.libcxx=libstdc++11
compiler.cppstd=gnu17
os=Linux

[buildenv]
CC=clang
CXX=clang++
CFLAGS=-fno-stack-protector -mno-outline-atomics
CXXFLAGS=-fno-stack-protector -mno-outline-atomics
EOF

WORKDIR /workspace

COPY ./conanfile.py /workspace
RUN conan install . \
  --output-folder=build-arm \
  --build=missing \
  -c tools.system.package_manager:mode=install && \
  rm -rf build-arm
