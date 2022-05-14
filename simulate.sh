#!/bin/bash
set -euxo pipefail

if [[ -z "${TRANSPORTSIM_INSTALL_DIR}" ]]; then
    if [[ -f transportsim_enable ]]; then
	source transportsim_enable
    else
        echo "Run this from the root project directory"
        exit 1
    fi
fi

./setup.sh && ./install/bin/game_simulator
