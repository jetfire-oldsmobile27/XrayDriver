SYSROOT_DIR := $(PWD)/aarch64-sysroot
ROOTFS_TARBALL := Manjaro-ARM-aarch64-latest.tar.gz
ROOTFS_URL := https://github.com/manjaro-arm/rootfs/releases/download/20250623/$(ROOTFS_TARBALL)

.PHONY: prepare clean

prepare:
	@if [ ! -f "$(SYSROOT_DIR)/usr/bin/qemu-aarch64-static" ]; then \
		echo "Скачиваем rootfs..."; \
		mkdir -p $(SYSROOT_DIR); \
		curl -L $(ROOTFS_URL) -o $(ROOTFS_TARBALL); \
		echo "Распаковываем rootfs..."; \
		tar xfp $(ROOTFS_TARBALL) -C $(SYSROOT_DIR); \
		rm $(ROOTFS_TARBALL); \
		echo "Устанавливаем dev-пакеты внутрь sysroot..."; \
		sudo pacman -Sy --noconfirm --root $(SYSROOT_DIR) \
			libxcb xcb-util xcb-util-wm xcb-util-keysyms \
			xcb-util-image xcb-util-renderutil \
			libxkbcommon libxkbcommon-x11; \
		echo "Устанавливаем binutils и xcb-proto внутрь sysroot..."; \
		sudo mount --bind /proc $(SYSROOT_DIR)/proc; \
		sudo mount --bind /sys $(SYSROOT_DIR)/sys; \
		sudo mount --bind /dev $(SYSROOT_DIR)/dev; \
		sudo chroot $(SYSROOT_DIR) pacman -Syu --noconfirm binutils xcb-proto libxcb; \
		sudo umount $(SYSROOT_DIR)/proc $(SYSROOT_DIR)/sys $(SYSROOT_DIR)/dev; \
	fi

	@echo "Создаём tools/build/user-config.jam..."
	@mkdir -p tools/build
	@echo 'using gcc : arm' > tools/build/user-config.jam
	@echo '  : /usr/xcc/aarch64-unknown-linux-gnu/bin/aarch64-unknown-linux-gnu-g++' >> tools/build/user-config.jam
	@echo '  : <compileflags>"--sysroot=/work/aarch64-sysroot"' >> tools/build/user-config.jam
	@echo '    <linkflags>"--sysroot=/work/aarch64-sysroot"' >> tools/build/user-config.jam
	@echo '  ;' >> tools/build/user-config.jam

	@echo "✅ Подготовка завершена"

clean:
	rm -rf $(SYSROOT_DIR)
	rm -f $(ROOTFS_TARBALL)
	rm -rf tools/build

build:
	docker build --progress=plain -t armv8-ubuntu18.04-conan-dockcross .
push:
	docker push armv8-ubuntu18.04-conan-dockcross