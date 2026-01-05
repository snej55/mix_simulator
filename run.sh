#!/usr/bin/bash
cmake --build build/ -j$(nproc)
cd build; ./main # run from binary directory
