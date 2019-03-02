#!/usr/bin/env bash
set -e

BUILDDIR=../build/cpvframework-tests

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}
cmake -DCMAKE_BUILD_TYPE=Debug \
	-DGTEST_SOURCE_DIR=/usr/src/gtest \
	../../tests
make V=1 --jobs=$(printf "%d\n4" $(nproc) | sort -n | head -1)

ASAN_OPTIONS="detect_leaks=1" \
	./CPVFrameworkTests --reactor-backend epoll


