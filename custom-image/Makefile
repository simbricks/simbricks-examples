# Copyright 2024 Max Planck Institute for Software Systems, and
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


PACKER_VERSION := 1.7.0

IMAGE := output-memcached/memcached

all: run

%.raw: %
	qemu-img convert -f qcow2 -O raw $< $@

$(IMAGE): packer image/image.pkr.hcl image/install.sh image/cleanup.sh
	rm -rf $(dir $@)
	mkdir -p input-memcached
	./packer build -var="syscores=`nproc`" image/image.pkr.hcl
	rm -rf input-memcached
	touch $@

packer:
	wget -O packer_$(PACKER_VERSION)_linux_amd64.zip \
	    https://releases.hashicorp.com/packer/$(PACKER_VERSION)/packer_$(PACKER_VERSION)_linux_amd64.zip
	unzip packer_$(PACKER_VERSION)_linux_amd64.zip
	rm -f packer_$(PACKER_VERSION)_linux_amd64.zip

.PHONY: clean
clean:
	rm -rf output-memcached input-memcached packer packer_cache out

.PHONY: run-qemu
run-qemu: $(IMAGE)
	simbricks-run --verbose --force --filter=memcached-qemu memcached.py

run-gem5: $(IMAGE).raw
	simbricks-run --verbose --force --filter=memcached-gem5 memcached.py
