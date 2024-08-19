#!/bin/bash
set -eux

export TVM_HOME=/root/tvm
export PYTHONPATH=$TVM_HOME/python:$TVM_HOME/vta/python
export VTA_HW_PATH=$TVM_HOME/3rdparty/vta-hw

apt-get update
apt-get -y install \
    build-essential \
    git \
    python-is-python3 \
    python3 \
    python3-cffi \
    python3-cloudpickle \
    python3-decorator \
    python3-dev \
    python3-matplotlib \
    python3-numpy \
    python3-opencv \
    python3-psutil \
    python3-pytest \
    python3-scipy \
    python3-setuptools \
    python3-typing-extensions \
    python3-tornado \
    libtinfo-dev \
    zlib1g-dev \
    build-essential \
    cmake \
    libedit-dev \
    libxml2-dev \
    llvm-dev

# build tvm
mkdir -p /root
git clone --recursive --branch main --depth 1 https://github.com/simbricks/tvm-simbricks.git /root/tvm
cd /root/tvm
cp 3rdparty/vta-hw/config/simbricks_pci_sample.json 3rdparty/vta-hw/config/vta_config.json
mkdir build
cp cmake/config.cmake build
cd build
cmake ..
make -j`nproc`

# add pre-tuned autotvm configurations
mkdir /root/.tvm
cd /root/.tvm
git clone https://github.com/tlc-pack/tophub.git tophub

# download files for Darknet model
export MODEL_NAME=yolov3-tiny
export REPO_URL=https://github.com/dmlc/web-data/blob/main/darknet/
export DARKNET_DIR=/root/darknet
mkdir -p $DARKNET_DIR
cd $DARKNET_DIR
wget -O ${MODEL_NAME}.cfg https://github.com/pjreddie/darknet/blob/master/cfg/${MODEL_NAME}.cfg?raw=true
wget -O ${MODEL_NAME}.weights https://pjreddie.com/media/files/${MODEL_NAME}.weights?raw=true
wget -O libdarknet2.0.so ${REPO_URL}lib/libdarknet2.0.so?raw=true
wget -O coco.names ${REPO_URL}data/coco.names?raw=true
wget -O arial.ttf ${REPO_URL}data/arial.ttf?raw=true
wget -O dog.jpg ${REPO_URL}data/dog.jpg?raw=true
wget -O eagle.jpg ${REPO_URL}data/eagle.jpg?raw=true
wget -O giraffe.jpg ${REPO_URL}data/giraffe.jpg?raw=true
wget -O horses.jpg ${REPO_URL}data/horses.jpg?raw=true
wget -O kite.jpg ${REPO_URL}data/kite.jpg?raw=true
wget -O person.jpg ${REPO_URL}data/person.jpg?raw=true
wget -O scream.jpg ${REPO_URL}data/scream.jpg?raw=true

# compile library for Darknet model with TVM
export PYTHONPATH=/root/tvm/python:/root/tvm/vta/python
python3 /root/tvm/vta/tutorials/frontend/deploy_detection-compile_lib.py $DARKNET_DIR
