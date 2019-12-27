#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/actix-web-benchmark
cd ${BUILDDIR}
cd release

./actix-web-benchmark
