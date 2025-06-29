#!/usr/bin/env bash
set -euo pipefail

# Скрипт для кросс-компиляции проекта XrayDriver под ARMv8 (aarch64) с использованием Dockcross и Conan 2.x.

# Параметры (можно изменить под свои нужды):
PROJECT_ROOT="${PWD}"
DOCKCROSS_REPO="https://github.com/dockcross/dockcross.git"
DOCKCROSS_DIR="./dockcross"
DOCKCROSS_IMAGE="linux-arm64-lts"
DOCKCROSS_SCRIPT="./dockcross-${DOCKCROSS_IMAGE}"
CONAN_PROFILE="armv8"
BUILD_PROFILE="x86_64-conan"
CONAN_PROFILES_DIR="$HOME/.conan2/profiles"
BUILD_DIR="build-armv8"

# 1. Клонируем Dockcross и генерируем скрипт
if [ ! -d "${DOCKCROSS_DIR}" ]; then
  echo "Клонируем dockcross в ${DOCKCROSS_DIR}..."
  git clone "${DOCKCROSS_REPO}" "${DOCKCROSS_DIR}"
fi
cd "${DOCKCROSS_DIR}"
if [ ! -x "${DOCKCROSS_SCRIPT}" ]; then
  echo "Генерируем скрипт ${DOCKCROSS_SCRIPT}..."
  docker run --rm dockcross/${DOCKCROSS_IMAGE} > "${DOCKCROSS_SCRIPT}"
  chmod +x "${DOCKCROSS_SCRIPT}"
fi
cd - >/dev/null

# 3.1 Монтирование sysroot 
SYSROOT_DIR="/home/jetpclaptop/workspace/projects/XrayDriver/aarch64-sysroot"
ROOTFS_TARBALL="Manjaro-ARM-aarch64-latest.tar.gz"
ROOTFS_URL="https://github.com/manjaro-arm/rootfs/releases/download/20250623/${ROOTFS_TARBALL}"

# if [ ! -d "${SYSROOT_DIR}/boot" ]; then
# echo "Монтирование sysroot(ОБЯЗАТЕЛЬНО РАЗМОНТИРОВАТЬ!)"
# fdisk -l ~/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img  
# sudo losetup -fP --offset $(( 999424 * 512 )) ~/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img  
# sudo mount /dev/loop0 ${SYSROOT_DIR}
# fi

# 3.2 Операции над sysroot

if [ ! -f "${SYSROOT_DIR}/usr/bin/qemu-aarch64-static" ]; then
  mkdir -p "${SYSROOT_DIR}"
  echo "Скачиваем Manjaro-ARM rootfs..."
  curl -L "${ROOTFS_URL}" -o "${ROOTFS_TARBALL}"
  echo "Распаковываем rootfs в ${SYSROOT_DIR}..."
  tar xfp "${ROOTFS_TARBALL}" -C "${SYSROOT_DIR}"
  rm "${ROOTFS_TARBALL}"

  echo "Устанавливаем dev-пакеты внутрь sysroot (pacman --root)..."
  sudo pacman -Sy --noconfirm --root "${SYSROOT_DIR}" \
  libxcb xcb-util xcb-util-wm xcb-util-keysyms \
  xcb-util-image xcb-util-renderutil \
  libxkbcommon libxkbcommon-x11

  echo "Установка binutils для aarch64..."
  sudo mount --bind /proc "${SYSROOT_DIR}/proc"
  sudo mount --bind /sys "${SYSROOT_DIR}/sys"
  sudo mount --bind /dev "${SYSROOT_DIR}/dev"
  sudo chroot "${SYSROOT_DIR}" pacman -Syu --noconfirm binutils
  sudo chroot "${SYSROOT_DIR}" pacman -Syu --noconfirm xcb-proto libxcb
  sudo umount "${SYSROOT_DIR}/proc" "${SYSROOT_DIR}/sys" "${SYSROOT_DIR}/dev"
fi

# 4. Кросс-компиляция проекта
echo "Запускаем сборку проекта в контейнере..."
mkdir -p tools/build
cat > tools/build/user-config.jam <<'EOF'
using gcc : arm
  : /usr/xcc/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-g++
  : <compileflags>"--sysroot=/work/aarch64-sysroot"
    <linkflags>"--sysroot=/work/aarch64-sysroot"
  ;
EOF

# запускаем контейнер и передаём всё тело сюда
"${PROJECT_ROOT}/${DOCKCROSS_DIR}/${DOCKCROSS_SCRIPT}" bash -c '
  set -euo pipefail

  echo "Working dir:"
  ls /usr/xcc/aarch64-unknown-linux-gnu/bin/
  ls /work/aarch64-sysroot
  sudo dpkg --add-architecture arm64
sudo apt update
sudo apt install -yq gcc-11 g++-11
sudo apt install -yq \
  libxcb1-dev libxcb-xkb-dev libxcb-icccm4-dev \
  libxcb-keysyms1-dev libxcb-image0-dev libxcb-render-util0-dev \
  libxkbcommon-dev libxkbcommon-x11-dev 
  export PKG_CONFIG_ALLOW_CROSS=1
  export SYSROOT=/work/aarch64-sysroot
  export CXXFLAGS="--sysroot=$SYSROOT"
  export CFLAGS="--sysroot=$SYSROOT"
  export LDFLAGS="--sysroot=$SYSROOT -L$SYSROOT/usr/lib -L$SYSROOT/lib"
  
  # Подменяем libstdc++.so, чтобы исключить зависимость от GLIBC_2.36
  export LD_LIBRARY_PATH="/work/aarch64-sysroot/usr/lib:/work/aarch64-sysroot/lib"

  # Проверка доступности компилятора
  $CXX --version
  sudo conan install . \
    --profile:host='"${CONAN_PROFILE}"' \
    --profile:build='"${BUILD_PROFILE}"' \
    --build=missing \
    -c tools.system.package_manager:mode=install \
    -c tools.system.package_manager:sudo=True \
    -c user.boost/*:stacktrace_addr2line_location=/work/aarch64-sysroot/usr/bin/addr2line \
    --output-folder='"${BUILD_DIR}"' &&

  cmake -S . -B '"${BUILD_DIR}"' \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE='"${BUILD_DIR}"'/conan_toolchain.cmake &&

  cmake --build '"${BUILD_DIR}"' -- -j"$(nproc)"
'


echo "Сборка завершена. Артефакты в ${PROJECT_ROOT}/${BUILD_DIR}."


