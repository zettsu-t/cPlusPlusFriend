FROM ubuntu:focal
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update
RUN apt-get install -y build-essential clang-12 lldb-12 lld-12 cmake curl gdb less make wget libboost-dev
RUN apt-get install -y python3 python3-pip
RUN pip3 install --no-input autopep8 natsort pylint pycodestyle

CMD tail -f < /dev/null
