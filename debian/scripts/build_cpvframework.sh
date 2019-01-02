#!/usr/bin/env bash
set -e

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	-DCMAKE_INSTALL_PREFIX=/usr \
	${SRCDIR}
make V=1

mkdir -p ${BUILDDIR_DEBUG}
cd ${BUILDDIR_DEBUG}
cmake -DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_INSTALL_PREFIX=/usr \
	${SRCDIR}
make V=1


