#!/bin/bash -eu

# build fuzzers

pushd fuzzing
cmake -DBOLOS_SDK=../BOLOS_SDK -Bbuild -H.
make -C build
mv ./build/fuzz_tx_parser "${OUT}"
mv ./build/libtxparser.so "${OUT}"
mv ./build/libtxparser.so.1 "${OUT}"
popd
