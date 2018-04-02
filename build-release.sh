#!/usr/bin/env bash
# break after command error
set -e

# build seastar
# use single core because it require so much memory (require atleast 3.5G for per core)
cd 3rd-party/seastar
if [ ! -f build-release.ninja ]; then
	./configure.py --mode=release --with libseastar.a --with seastar.pc
	mv build.ninja build-release.ninja
fi
ninja -j1 -f build-release.ninja
cd ../..

# build cpv framework
mkdir -p bin/release
cd bin/release
cmake -DCMAKE_BUILD_TYPE=Release ../../src
make V=1
cd ../..

