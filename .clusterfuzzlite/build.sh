#!/bin/bash -eu

# build fuzzers

pushd fuzzing
cmake -DBOLOS_SDK=../BOLOS_SDK -Bbuild -H. -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCODE_COVERAGE=1
cmake --build build --target fuzz_tx_parser
mv ./build/fuzz_tx_parser "${OUT}/fuzz_tx_parser"
popd
