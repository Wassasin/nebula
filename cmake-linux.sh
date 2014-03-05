#!/bin/bash
set -e
set +x

rm -rf build
mkdir -p build
pushd build
CC="clang" CXX="clang++" cmake ..
popd

