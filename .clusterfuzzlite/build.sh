#!/bin/bash -eu

# build fuzzers

pushd fuzzing
cmake -DBOLOS_SDK=../BOLOS_SDK -Bbuild -H.
make -C build
mv ./build/fuzz_tx_parser "${OUT}/fuzz_tx_parser"
mv ./build/libtxparser.so "${OUT}/libtxparser.so"
mv ./build/libtxparser.so.1 "${OUT}/libtxparser.so.1"
ls -l "${OUT}"
popd
