# ubuntu ver: 14.04, 16.04, 18.04, 20.04, 21.10, 22.04
FROM ubuntu:20.04

# https://llvm.org/
# https://apt.llvm.org/

ARG VERSION="14"
ARG WORKDIR="test"

RUN mkdir /clang && \
    mkdir /$WORKDIR && \
    umask 0000

RUN apt-get update && apt-get install -y lsb-release wget make software-properties-common && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /clang
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh ${VERSION} all

# clang-xxxという名前で設定されるので, clangという名前でシンボリックリンクを作成する
RUN ln -sf /usr/bin/clang-${VERSION} /usr/bin/clang && \
    ln -sf /usr/bin/clang++-${VERSION} /usr/bin/clang++

WORKDIR /$WORKDIR
ENV COMPILER="clang"
ENV TZ=Asia/Tokyo
