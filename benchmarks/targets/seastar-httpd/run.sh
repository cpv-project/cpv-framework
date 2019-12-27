#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/seastar-httpd-benchmark

cd ${BUILDDIR}

./httpd
