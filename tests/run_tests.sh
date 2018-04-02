#!/usr/bin/env bash
# break after command error
set -e

# build cql driver
cd ..
bash build-debug.sh
cd tests

# build tests
mkdir -p ../bin/tests
cd ../bin/tests
cmake -DCMAKE_BUILD_TYPE=Debug ../../tests
make
cd ../../tests

# run tests
cd ../bin/tests
echo "leak:lt__malloc" > "suppressions.txt"
LSAN_OPTIONS="suppressions=suppressions.txt" ./CPVFrameworkTests
cd ../../tests

