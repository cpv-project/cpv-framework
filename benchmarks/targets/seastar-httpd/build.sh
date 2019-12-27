#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/seastar-httpd-benchmark

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}

g++-9 $(pkg-config --cflags seastar) ../../benchmarks/targets/seastar-httpd/httpd.cpp $(pkg-config --libs seastar) -O3 -o httpd
