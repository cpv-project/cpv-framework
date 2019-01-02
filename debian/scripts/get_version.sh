#!/usr/bin/env bash
cat $(dirname $0)/../changelog | head -n 1 | python3 -c "import re;print(re.search('\((.+)\)', input()).groups()[0])"
