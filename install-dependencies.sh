#!/usr/bin/env bash
# break after command error
set -e

# install seastar dependencies
cd 3rd-party/seastar
sudo bash install-dependencies.sh

# install cpv framework dependencies
# nothing for now

