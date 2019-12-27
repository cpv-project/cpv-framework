#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/actix-web-benchmark
mkdir -p ${BUILDDIR}

. $HOME/.cargo/env
CARGO_TARGET_DIR=../../../build/actix-web-benchmark cargo build --release
