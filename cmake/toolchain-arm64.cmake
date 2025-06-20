# toolchain-arm64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Компиляторы
set(CMAKE_C_COMPILER   aarch64-linux-gnu-gcc  CACHE STRING "" FORCE)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++  CACHE STRING "" FORCE)

# Пути к библиотекам (sysroot)
 set(CMAKE_SYSROOT ~/aarch64-sysroot)

# Обязательно:
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

