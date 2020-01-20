#!/usr/bin/env bash
set -e
cat Cargo.lock | grep "hyper " | head -n 1 | awk '{ print $2 }'
