#!/bin/bash

pushd .
cd cpp
clang-tidy *.cpp -checks=perf\* -- -std=gnu++17 -I /usr/include -isystem ./googletest/googletest/include
clang-format -i *.cpp
make
make run
./thread_safety --threads 2 --trials 3 --dict_size 1000 --doc_size 1000000 --strategy fetch_add
./thread_safety --threads 2 --trials 3 --dict_size 1000 --doc_size 1000000 --strategy compare_and_swap
timeout 10 ./thread_safety --threads 2 --trials 10 --dict_size 1000 --doc_size 1000000
./std_execution --trial 2 --size 10000000 --max 200 --target for_each
./std_execution --trial 2 --size 1000000 --max 200 --target sort
./std_execution --trial 1 --size 100000 --max 200
popd

pushd .
cd rust/par_iter
cargo fmt
cargo clippy
cargo test
cargo run -- --trial 2 --size 10000000 --max 200 --target double
cargo run -- --trial 2 --size 1000000 --max 200 --target sum
cargo run -- --trial 1 --size 100000 --max 200
popd
