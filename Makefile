.PHONY: run
run: build
	cd build && ./diye || cd ..

.PHONY: build
build:
	@cmake --build build/

.PHONY: debug
debug:
	@renderdoccmd capture -d build ./build/diye

.PHONY: init
init: clean
	@mkdir -p build/
	@cmake -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Debug -S . -B build -G Ninja

.PHONY: shaders
shaders:
	@cd src/shaders/dummy && ninja

.PHONY: clean
clean:
	@rm -rf build/

.PHONY: assets
assets:
	@echo "Copy assets/"
	@cp assets/* build/
	@echo "Download viking demo"
	@curl --output build/viking_room.obj https://vulkan-tutorial.com/resources/viking_room.obj
	@curl --output build/viking_room-24.png https://vulkan-tutorial.com/resources/viking_room.png
	@echo "Make sure to convert viking_room.png to 32-bit with alpha channel"
	@convert build/viking_room-24.png -alpha on -depth 8 build/viking_room.png
	@file build/viking_room.png
