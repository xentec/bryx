export CC=clang
export CXX=clang++

MODE=Release

all: checkout create build copy

checkout:
    git submodule update --init
	git checkout working

create:
    mkdir -p build

.PHONY: build
build:
    cd build && \
	cmake -DCMAKE_BUILD_TYPE=$(MODE) .. && \
	cmake --build . --target all

copy:
    mkdir -p bin && cp -v build/bryx bin/client_01

.PHONY: clean
clean:
    rm -r build
	git reset --hard master
