#!/usr/bin/env bash
set -e
dpkg-query --showformat='${Version}\n' --show seastar
