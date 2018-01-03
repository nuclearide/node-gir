#!/bin/bash
set -e
BASE_DIR=$(git rev-parse --show-toplevel)
cd $BASE_DIR;
./node_modules/.bin/cmake-js configure
cd build
make clang-format
