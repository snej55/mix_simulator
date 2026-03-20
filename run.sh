#!/usr/bin/bash
# for debug:
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
#       -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g" \
#       -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build/ -j$(nproc)
cd build; ./main # run from binary directory
