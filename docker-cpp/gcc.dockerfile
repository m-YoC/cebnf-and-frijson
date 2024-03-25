FROM gcc:11.2
# https://hub.docker.com/_/gcc/

RUN apt-get update && apt-get install -y --no-install-recommends wget make && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


ARG WORKDIR="test"

RUN mkdir /$WORKDIR && mkdir /$WORKDIR/logs && mkdir /$WORKDIR/hello-world && umask 0000

WORKDIR /$WORKDIR

COPY ./shell/ /$WORKDIR/
COPY ./hello-world/ /$WORKDIR/hello-world/

ENV COMPILER="gcc"
ENV TZ=Asia/Tokyo
