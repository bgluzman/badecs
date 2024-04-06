#!/usr/bin/env bash

SCRIPT_DIR="$(dirname "$0")"
cd "$SCRIPT_DIR/.." || exit 1
mkdir -p coverage
gcovr --filter include/badecs --html-details coverage/coverage.html