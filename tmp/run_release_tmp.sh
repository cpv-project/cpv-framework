#!/usr/bin/env bash
set -e

BUILDDIR=../build/cpvframework-tmp

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}
cmake -DCMAKE_BUILD_TYPE=Release \
	../../tmp
make V=1 --jobs=$(printf "%d\n4" $(nproc) | sort -n | head -1)

./CPVFrameworkTmp --task-quota-ms=20



