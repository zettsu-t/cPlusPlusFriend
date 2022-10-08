# docker build . -t rust
# docker run -d -v /path/to/proj:/home/work --cap-add=SYS_PTRACE --security-opt seccomp=unconfined rust
FROM rust
RUN rustup component add rustfmt clippy llvm-tools-preview
RUN cargo install cargo-edit cargo-profiler cargo-tarpaulin rustfilt
RUN apt-get update && apt-get install -y build-essential clang clang-format clang-tidy cmake doxygen gdb lcov less valgrind libboost-all-dev libpng-dev
RUN echo PS1='"\[\e[0;36m\][\u@\h \w]\\$\[\e[0m\] "' >> /root/.bashrc
