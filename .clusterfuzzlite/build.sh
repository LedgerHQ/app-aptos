#!/bin/bash -eu

# build fuzzers

pushd fuzzing
cmake -DBOLOS_SDK=../BOLOS_SDK -Bbuild -H.
make -C build
ls build
mv ./build/fuzz_tx_parser "${OUT}"
popd
