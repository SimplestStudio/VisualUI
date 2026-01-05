#!/bin/bash

set -e
cd "$(dirname "$0")/.."

for conf in Debug Release; do
    cmake -S . -B "lib/build/$conf" -G Ninja -DCMAKE_BUILD_TYPE="$conf"
    cmake --build "lib/build/$conf"
    cmake --install "lib/build/$conf"
done
