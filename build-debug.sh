#!/usr/bin/env bash
# break after command error
set -e

# build seastar
# use single core because it require so much memory (require atleast 3.5G for per core)
cd 3rd-party/seastar
if [ ! -f build-debug.ninja ]; then
	./configure.py --mode=debug --with libseastar.a --with seastar.pc --with fmt/fmt/libfmt.a
	mv build.ninja build-debug.ninja
fi
ninja -j1 -f build-debug.ninja
cd ../..

# build cpv framework
mkdir -p bin/debug
cd bin/debug
cmake -DCMAKE_BUILD_TYPE=Debug ../../src
make V=1
cd ../..

