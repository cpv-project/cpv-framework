#!/usr/bin/env bash
# break after command error
set -e

# clean seastar
cd 3rd-party/seastar
rm -fv build*.ninja
rm -rfv build
cd ../..

# clean cpv framework
rm -rfv bin

