#!/usr/bin/env bash
set -e

BUILDDIR=../../../build/seastar-httpd-benchmark

cd ${BUILDDIR}

./httpd \
	--task-quota-ms=20 \
	--smp=$(nproc) \
	--reactor-backend=epoll

