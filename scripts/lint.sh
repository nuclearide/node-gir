#!/bin/bash
set -e
BASE_DIR=$(git rev-parse --show-toplevel)
cd $BASE_DIR/build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ../
$BASE_DIR/scripts/run-clang-tidy.py -header-filter='*' -checks='*' -fix
