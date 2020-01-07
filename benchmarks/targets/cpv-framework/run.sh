#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/cpvframework-benchmark

cd ${BUILDDIR}

./CPVFrameworkBenchmark \
	--task-quota-ms=20 \
	--smp=$(nproc) \
	--reactor-backend=epoll

