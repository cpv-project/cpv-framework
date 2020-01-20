#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/hyper-benchmark
mkdir -p ${BUILDDIR}

. $HOME/.cargo/env
CARGO_TARGET_DIR=../../../build/hyper-benchmark cargo build --release
