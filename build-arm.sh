
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export AR=aarch64-linux-gnu-ar
export RANLIB=aarch64-linux-gnu-ranlib

conan install . \
  --output-folder=build-arm \
  --build=missing \
  --profile:build=default \
  --profile:host=./.profiles/arm64_2 

cmake -S . -B build-arm \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm64.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=$(realpath build-arm)

cmake --build build-arm -j$(nproc)

#to make sysroot arm:
# fdisk -l ~/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img 
#Disk /home/jetpclaptop/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img: 5,73 GiB, 6156189696 bytes, 12023808 sectors
#Units: sectors of 1 * 512 = 512 bytes
#Sector size (logical/physical): 512 bytes / 512 bytes
#I/O size (minimum/optimal): 512 bytes / 512 bytes
#Disklabel type: gpt
#Disk identifier: 8F621CA6-15CD-4C2C-A41C-275375804B23

#Device                                                                 Start      End  Sectors  Size Type
#/home/jetpclaptop/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img1   2048   999423   997376  487M Micr
#/home/jetpclaptop/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img2 999424 12021759 11022336  5,3G Linu
# make offset by .img1(+1): ((999424*512)) and make loop:
# > sudo losetup -fP --offset $(( 999424 * 512 )) ~/Downloads/Manjaro-ARM-kde-plasma-generic-23.02.img
# sudo mount /dev/loop0 ~/aarch64-sysroot


#disable after all:
# sudo umount ~/aarch64-sysroot
# sudo losetup -d /dev/loop0