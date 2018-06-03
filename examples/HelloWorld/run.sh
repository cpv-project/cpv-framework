#!/usr/bin/env bash
PKGCONFIG_PATH="../../bin/debug/cpvframework.pc"
BIN_PATH="../../bin/example"
g++ $(pkg-config --cflags $PKGCONFIG_PATH) Main.cpp $(pkg-config --libs $PKGCONFIG_PATH) -o $BIN_PATH && $BIN_PATH

