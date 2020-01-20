#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/hyper-benchmark
cd ${BUILDDIR}
cd release

./hyper-benchmark
