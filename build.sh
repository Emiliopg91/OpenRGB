#!/bin/sh

set -e

rm -rf build squashfs-root
mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
cmake --build . -j$(nproc)

cd ..

chmod +x scripts/build-udev-rules.sh .
./scripts/build-udev-rules.sh .