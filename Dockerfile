# Pull base image.
FROM ubuntu:16.04
#FROM alpine:3.7

# Define workdir
WORKDIR /root

# Install some tools: gcc build tools, unzip, etc
RUN apt-get update && \
    apt-get -y upgrade && \
    apt-get -y install curl build-essential unzip cmake

# Download and install libsodium
# https://download.libsodium.org/doc/

# Download & extract & make libsodium
# Move libsodium build
RUN \
    mkdir -p /tmpbuild/libsodium && \
    cd /tmpbuild/libsodium && \
    curl -L https://download.libsodium.org/libsodium/releases/libsodium-1.0.17.tar.gz -o libsodium-1.0.17.tar.gz && \
    tar xfvz libsodium-1.0.17.tar.gz && \
    cd /tmpbuild/libsodium/libsodium-1.0.17/ && \
    ./configure && \
    make && make check && \
    make install && \
    mv src/libsodium /usr/local/ && \
    rm -Rf /tmpbuild/

WORKDIR /usr/src/amp11/src
RUN \
    curl -L https://github.com/tyurek/relic/archive/master.zip -o master.zip && \
    unzip master.zip && \
    mkdir relic-target && \
    cd relic-target && \
    cmake ../relic-master/ 
RUN cd relic-target && \
    make
RUN cd relic-target && \
    make install
RUN ldconfig
# Define default command
CMD ["bash"]