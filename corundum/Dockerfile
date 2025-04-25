ARG REGISTRY=
ARG TAG=:latest

FROM ${REGISTRY}simbricks/simbricks-fullsims${TAG} AS corelib

USER root

COPY . /corundum_src

RUN cd /tmp \
    && apt-get update \
    && apt-get install help2man \
    && git clone -b v5.026 https://github.com/verilator/verilator \
    && cd verilator \
    && autoupdate \
    && autoconf \
    && ./configure \
    && make -j`nproc` \
    && make install 


WORKDIR /corundum_src
RUN pip install --no-cache-dir --force-reinstall -r requirements.txt \
    && cd /simbricks \
    && patch -p1 < /corundum_src/kernel-config.patch \
    && make -j `nproc` build-images-min COMPRESSED_IMAGES=true \
    && cd /corundum_src/corundum \
    && patch -p1 < /corundum_src/corundum-verilog.patch \
    && cd /corundum_src \
    && make all

USER simbricks