FROM dockcross/linux-arm64-lts

ENV CONAN_PROFILE="armv8"
ENV BUILD_PROFILE="x86_64-conan"
ENV BUILD_DIR="build-armv8"

COPY . /work
WORKDIR /work
RUN echo "Working dir: ${PWD}" && \
  ls /usr/xcc/aarch64-unknown-linux-gnu/bin/ &&\
  ls /work/aarch64-sysroot &&\
  sudo dpkg --add-architecture arm64 &&\
  sudo apt update &&\
  sudo apt install -yq gcc-11 g++-11 &&\
  sudo apt install -yq\
  libxcb1-dev libxcb-xkb-dev libxcb-icccm4-dev \
  libxcb-keysyms1-dev libxcb-image0-dev libxcb-render-util0-dev \
  libxkbcommon-dev libxkbcommon-x11-dev &&\
  export PKG_CONFIG_ALLOW_CROSS=1 &&\
  export SYSROOT=/work/aarch64-sysroot &&\
  export CXXFLAGS="--sysroot=$SYSROOT" &&\
  export CFLAGS="--sysroot=$SYSROOT" &&\
  export LDFLAGS="--sysroot=$SYSROOT -L$SYSROOT/usr/lib -L$SYSROOT/lib" &&\
  export LD_LIBRARY_PATH="/work/aarch64-sysroot/usr/lib:/work/aarch64-sysroot/lib" &&\
  sudo conan install . \
    --profile:host="${CONAN_PROFILE}" \
    --profile:build="${BUILD_PROFILE}" \
    --build=missing \
    -c tools.system.package_manager:mode=install \
    -c tools.system.package_manager:sudo=True \
    -c user.boost/*:stacktrace_addr2line_location=/work/aarch64-sysroot/usr/bin/addr2line \
    --output-folder="${BUILD_DIR}"
  
RUN rm -rf ${BUILD_DIR}



