FROM gcc:11.2
# https://hub.docker.com/_/gcc/

ARG WORKDIR="test"

RUN mkdir /$WORKDIR && umask 0000

RUN apt-get update && apt-get install -y wget make && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /$WORKDIR
ENV COMPILER="gcc"
ENV TZ=Asia/Tokyo
