# ubuntu ver: 14.04, 16.04, 18.04, 20.04, 21.10, 22.04
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y --no-install-recommends lsb-release wget make software-properties-common && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


# ------ Install clang ------

# https://llvm.org/
# https://apt.llvm.org/

ARG CLANG_VERSION="14"

RUN mkdir /clang && umask 0000

WORKDIR /clang
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh ${CLANG_VERSION} all

# clang-xxxという名前で設定されるので, clangという名前でシンボリックリンクを作成する
RUN ln -sf /usr/bin/clang-${CLANG_VERSION} /usr/bin/clang && \
    ln -sf /usr/bin/clang++-${CLANG_VERSION} /usr/bin/clang++


# ------ Working directory settings ------

ARG WORKDIR="test"

RUN mkdir /$WORKDIR && \
    mkdir /$WORKDIR/logs && mkdir /$WORKDIR/hello-world && \
    umask 0000

WORKDIR /$WORKDIR

COPY ./shell/ /$WORKDIR/
COPY ./hello-world/ /$WORKDIR/hello-world/

ENV COMPILER="clang"
ENV TZ=Asia/Tokyo
