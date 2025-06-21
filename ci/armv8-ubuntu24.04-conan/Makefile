prepare:
	docker run --rm --privileged multiarch/qemu-user-static --reset -p yes && \
	sudo pacman -S docker-buildx && \
	docker buildx create --use && \
    docker run --rm --privileged tonistiigi/binfmt --install all
build:
	docker buildx build --platform=linux/arm64/v8 -t armv8-ubuntu24.04-conan .
push:
	docker push armv8-ubuntu24.04-conan