#!/usr/bin/env bash
set -e
dotnet --info | grep AspNetCore | awk '{print $2}' | sort -r | head -n 1
