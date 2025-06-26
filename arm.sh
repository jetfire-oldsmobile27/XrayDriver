#!/usr/bin/env bash
set -euo pipefail

# Скрипт для кросс-компиляции проекта XrayDriver под ARMv8 (aarch64) с использованием Dockcross и Conan 2.x.

# Параметры (можно изменить под свои нужды):
PROJECT_ROOT="/home/wolfdale/workspace/projects/XrayDriver"
DOCKCROSS_REPO="https://github.com/dockcross/dockcross.git"
DOCKCROSS_DIR="./dockcross"
DOCKCROSS_IMAGE="linux-arm64"
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

# 3. Создаём профиль Conan для ARMv8 на хосте, если не существует
PROFILE_PATH="${CONAN_PROFILES_DIR}/${CONAN_PROFILE}"
if [ ! -f "${PROFILE_PATH}" ]; then
  echo "Создаём профиль Conan '${CONAN_PROFILE}'..."
  mkdir -p "${CONAN_PROFILES_DIR}"
  cat > "${PROFILE_PATH}" <<EOF
[settings]
os=Linux
arch=armv8
arch_build=x86_64
compiler=gcc
compiler.version=12
compiler.libcxx=libstdc++11
build_type=Release

[env]
CC=aarch64-linux-gnu-gcc
CXX=aarch64-linux-gnu-g++
EOF
fi

# 4. Кросс-компиляция проекта
echo "Запускаем сборку проекта в контейнере..."

# смонтируем ~/.conan2, сам проект и выставим рабочую директорию
export DOCKER_RUN_ARGS="\
  -v ${HOME}/.conan2:${HOME}/.conan2 \
  -v ${PROJECT_ROOT}:/project \
  -w /project"

# запускаем контейнер и передаём всё тело сюда
"${PROJECT_ROOT}/${DOCKCROSS_DIR}/${DOCKCROSS_SCRIPT}" bash -c '
  set -euo pipefail

  echo "Working dir: $(${CC})"
  sudo dpkg --add-architecture arm64
  ls /usr/xcc &&
sudo apt update
sudo apt install gcc g++
which gcc
  sudo conan install . \
    --profile:host='"${CONAN_PROFILE}"' \
    --profile:build='"${BUILD_PROFILE}"' \
    --build=missing \
    -c tools.system.package_manager:mode=install \
    -c tools.system.package_manager:sudo=True \
    --output-folder='"${BUILD_DIR}"' &&

  cmake -S . -B '"${BUILD_DIR}"' \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE='"${BUILD_DIR}"'/conan_toolchain.cmake &&

  cmake --build '"${BUILD_DIR}"' -- -j"$(nproc)"
'


echo "Сборка завершена. Артефакты в ${PROJECT_ROOT}/${BUILD_DIR}."


