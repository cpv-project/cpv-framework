#!/usr/bin/env bash
BIN_PATH="../../build/example-hello-world/Main"
mkdir -p $(dirname "${BIN_PATH}")
g++-9 $(pkg-config --cflags seastar) \
	$(pkg-config --cflags cpvframework) \
	Main.cpp \
	$(pkg-config --libs seastar) \
	$(pkg-config --libs cpvframework) \
	-o ${BIN_PATH} && \
	${BIN_PATH} --reactor-backend epoll


