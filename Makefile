MODE=Release

all: checkout create build copy

checkout:
	git submodule update --init

create:
	mkdir -p build

.PHONY: build
build:
	cd build && \
	cmake -DCMAKE_BUILD_TYPE=$(MODE) .. && \
	cmake --build . --target all

copy:
	mkdir -p bin && cp -v build/bryx bin/client01
	rm -rf build

.PHONY: clean
clean:
	rm -rf bin
