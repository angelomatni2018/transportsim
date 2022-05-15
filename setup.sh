#!/bin/bash
set -euxo pipefail

if [[ -z "${TRANSPORTSIM_INSTALL_DIR:-""}" ]]; then
    if [[ -f transportsim_enable ]]; then
	source transportsim_enable
    else
        echo "Run this from the root project directory"
        exit 1
    fi
fi

mkdir -p build
cd build/
cmake -DCMAKE_INSTALL_PREFIX="$TRANSPORTSIM_INSTALL_DIR" -DCMAKE_BUILD_TYPE=Debug ../
make -j4
make install
cd ../
