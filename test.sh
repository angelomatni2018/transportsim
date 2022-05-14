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
export TRANSPORTSIM_BUILD_TESTS=true
cmake -DCMAKE_INSTALL_PREFIX="$TRANSPORTSIM_INSTALL_DIR" -DCMAKE_BUILD_TYPE=Debug ../
make -j4 && make install
cd ../

echo "Running tests"
find ${TRANSPORTSIM_INSTALL_DIR}/bin -name "*_test" | xargs -I {} sh -c 'echo "Executing {}" ; {}'