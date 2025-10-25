#!/usr/bin/bash
cmake --build build/ -j14
cd build; ./main # run from binary directory
