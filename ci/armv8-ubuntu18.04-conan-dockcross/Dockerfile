FROM archlinux:multilib-devel-20250622.0.370030

RUN  pacman -Syu && mkdir /cache
COPY . /cache
WORKDIR /cache
RUN chmod +x arm.sh && ./arm.sh &&  rm -rf build-armv8



