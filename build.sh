#!/usr/bin/env bash
set -e

VERSION=$(sh ./debian/scripts/get_version.sh)
BUILDDIR=build/cpvframework-${VERSION}
TYPE=$1

mkdir -p ${BUILDDIR}
cd ${BUILDDIR}
cp -rf ../../debian .
cp -rf ../../src .
cp -rf ../../include .
cp -f ../../LICENSE .

if [ "${TYPE}" = "local" ]; then
	debuild
elif [ "${TYPE}" = "ppa" ]; then
	debuild -S -sa
else
	echo "build.sh {local,ppa}"
	exit 1
fi


