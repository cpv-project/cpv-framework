#!/usr/bin/env bash
set -e

BUILDDIR=../build/cpvframework-tests

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}
cmake -DCMAKE_BUILD_TYPE=Debug \
	-DHTTP_SERVER_1_IP=127.0.0.1 \
	-DHTTP_SERVER_1_PORT=8100 \
	-DHTTP_SERVER_2_IP=0.0.0.0 \
	-DHTTP_SERVER_2_PORT=8101 \
	-DGTEST_SOURCE_DIR=/usr/src/gtest \
	../../tests
make V=1 --jobs=$(printf "%d\n4" $(nproc) | sort -n | head -1)

ASAN_OPTIONS="detect_leaks=1" \
	./CPVFrameworkTests --task-quota-ms=20


