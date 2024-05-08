#!/bin/bash -eux

set -eux
nproc
apt-get update
apt-get -y install build-essential libevent-dev memcached libevent-dev \
  autoconf automake libtool
systemctl disable memcached.service

cd /tmp
wget https://launchpad.net/libmemcached/1.0/1.0.18/+download/libmemcached-1.0.18.tar.gz
tar xf libmemcached-1.0.18.tar.gz
cd libmemcached-1.0.18
sed -i -e 's:include tests/include.am::' Makefile.am
autoreconf -i
./configure --enable-memaslap --disable-dtrace --prefix=/usr --enable-static \
    --disable-shared \
    CXXFLAGS='-fpermissive -fcommon -pthread' \
    CFLAGS='-fpermissive -fcommon -pthread' \
    LDFLAGS='-fcommon -pthread' || (cat config.log ; exit 1)
make -j`nproc`
make -j`nproc` install

cd /tmp
rm -rf libmemcached-1.0.18 libmemcached-1.0.18.tar.gz
