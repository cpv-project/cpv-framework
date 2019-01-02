#!/usr/bin/env bash
set -e

cd ${BUILDDIR}
DESTDIR=${ROOTDIR} make install V=1

cd ${BUILDDIR_DEBUG}
DESTDIR=${ROOTDIR_DEBUG} make install V=1

cd ${ROOTDIR}
cp -f ${ROOTDIR_DEBUG}/usr/lib/x86_64-linux-gnu/libCPVFramework.so \
	${ROOTDIR}/usr/lib/x86_64-linux-gnu/libCPVFramework_debug.so
cp -f ${ROOTDIR_DEBUG}/usr/lib/x86_64-linux-gnu/pkgconfig/cpvframework.pc \
	${ROOTDIR}/usr/lib/x86_64-linux-gnu/pkgconfig/cpvframework-debug.pc
strip ${ROOTDIR}/usr/lib/x86_64-linux-gnu/*.so

sed -i "s#-lCPVFramework#-lCPVFramework_debug#g" \
  ${ROOTDIR}/usr/lib/x86_64-linux-gnu/pkgconfig/cpvframework-debug.pc

patchelf --set-soname libCPVFramework_debug.so ${ROOTDIR}/usr/lib/x86_64-linux-gnu/libCPVFramework_debug.so


