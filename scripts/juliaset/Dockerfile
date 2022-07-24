# docker build . -t rust
# docker run -d -v /path/to/proj:/home/work --cap-add=SYS_PTRACE --security-opt seccomp=unconfined rust
FROM rust
RUN rustup component add rustfmt clippy llvm-tools-preview
RUN cargo install cargo-edit cargo-tarpaulin rustfilt
