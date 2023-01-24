help:
	@just --list

build-with-config config:
	@mkdir -p build 
	@cd build && cmake ..
	@cmake --build ./build --config {{config}} --target raylib-cpp-cmake-template -j 10 --

build-debug:
	@just build-with-config Debug

build-release:
	@just build-with-config Release

clean:
	@rm -rf build || true
	@rm -rf out || true

build-web:
	#!/usr/bin/env bash
	sudo emsdk activate latest
	source "/usr/lib/emsdk/emsdk_env.sh"
	mkdir -p build-emc 
	
	# Ensure asset folder is copied
	rm -rf build-emc/assets || true
	cp -R assets build-emc/assets

	cd build-emc
	emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-s USE_GLFW=3" -DCMAKE_EXECUTABLE_SUFFIX=".html"
	emmake make
