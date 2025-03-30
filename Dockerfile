FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

ENV TZ=Asia/Tokyo
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        libgmp-dev \
        autoconf \
        automake \
        libtool \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work

COPY . /work/


CMD ["bash"]
