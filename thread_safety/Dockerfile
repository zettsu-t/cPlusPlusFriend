FROM ubuntu:jammy
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y build-essential clang clang-format clang-tidy cmake curl gdb lcov less make patch unzip wget libxt-dev
RUN apt-get install -y libboost-all-dev libtbb-dev
RUN curl https://sh.rustup.rs -sSf | bash -s -- -y

CMD tail -f < /dev/null
