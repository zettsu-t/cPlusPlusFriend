#!/bin/bash

pushd .
cd cpp
clang-tidy *.cpp -checks=perf\* -- -std=gnu++17 -I /usr/include -isystem ./googletest/googletest/include
clang-format -i *.cpp
popd

pushd .
cd rust/par_iter
cargo fmt
cargo clippy
popd
