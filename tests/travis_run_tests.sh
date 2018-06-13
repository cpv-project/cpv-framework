#!/usr/bin/env bash
# run tests in docker on travis ci
# reason:
# - build all things on ubuntu 14.04 is hard
# - even if the build is successful, the asan on 14.04 is buggy

# break after command error
set -e

# prepare docker
docker pull ubuntu:18.04

# create container and run
Command=$(cat <<"EOF"
  apt-get update && \
  apt-get install sudo -y && \
  cd /project && \
  sh install-dependencies.sh && \
  cd tests && \
  sh run_tests.sh
EOF
)
docker run \
  -it \
  --rm \
  --privileged \
  --net "host" \
  --volume "$(realpath ../):/project" \
  ubuntu:18.04 \
  bash -c "$Command"
