#!/usr/bin/env bash
ulimit -n 65536
ulimit -a
echo
python3 _runner.py
echo "done"

