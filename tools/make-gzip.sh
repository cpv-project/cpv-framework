#!/usr/bin/env bash
# Generate pre-compressed gzip files of static files for HttpServerRequestStaticFileHandler
set -e
find . -type f -not -name '*.gz' | xargs gzip -fkn

