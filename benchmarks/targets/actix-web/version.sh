#!/usr/bin/env bash
set -e
cat Cargo.lock | grep "actix-web " | head -n 1 | awk '{ print $2 }'
