# Copyright 2021 Max Planck Institute for Software Systems, and
# National University of Singapore
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

ARG REGISTRY=
ARG TAG=:latest

FROM ${REGISTRY}simbricks/simbricks-executor${TAG}

USER root

# install newer verilator version in order to later on verilate corundum
RUN cd /tmp \
    && apt-get update \
    && apt-get install help2man \
    && git clone -b v5.032 https://github.com/verilator/verilator \
    && cd verilator \
    && autoupdate \
    && autoconf \
    && ./configure \
    && make -j`nproc` \
    && make install \
    && rm -rf /tmp/verilator

USER simbricks

COPY --chown=simbricks . /corundum_src
ENV PYTHONPATH="/corundum_src"

# Build linux image that can be used with corundum
RUN <<EOF
# Download the kernel that is used by the base image in order to build the corundum driver
cd /simbricks
make prep-mods

# Build corundum driver and adapter
cd /corundum_src/corundum
patch -p1 < /corundum_src/corundum-verilog.patch
cd /corundum_src
make -j `nproc` all

EOF

WORKDIR /simbricks