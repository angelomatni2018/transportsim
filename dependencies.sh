#!/bin/bash

mkdir -p include/external
if ! [[ -f include/external/catch.hpp ]]; then
    curl -L https://github.com/catchorg/Catch2/releases/download/v2.13.9/catch.hpp --output include/external/catch.hpp
fi

sudo apt-get update
sudo apt-get install cmake clang-format libsfml-dev=2.5.1+dfsg-1build1 libspdlog-dev=1:1.5.0-1 -y

# TODO: Try and get catch2 v3 working eventually:
# mkdir -p external
# cd external
# #rm -rf Catch2
# git clone https://github.com/catchorg/Catch2
# cd Catch2
# rm -rf build
# mkdir build
# cmake -Bbuild -H. -DBUILD_TESTING=OFF
# cmake --build build --target $TRANSPORTSIM_INSTALL_DIR