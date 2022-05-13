#!/bin/bash
set -euxo pipefail

./setup.sh > /dev/null
echo "Running tests"
find ./install/bin -name "*_test" | xargs -I {} sh -c 'echo "Executing {}" ; {}'