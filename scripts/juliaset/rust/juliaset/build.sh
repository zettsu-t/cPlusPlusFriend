#!/bin/bash
cargo build
cargo run
cargo test
cargo doc --document-private-items
cargo clippy
cargo fmt
cargo tarpaulin
cargo build --release
cargo bench
cargo profiler callgrind --release
