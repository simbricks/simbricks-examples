PACKER_VERSION := 1.7.0

IMAGE := output-tvm/tvm
VTA_DIR := vta_src/simbricks
VTA := $(VTA_DIR)/vta_simbricks

all: experiment_requirements

%.raw: %
	qemu-img convert -f qcow2 -O raw $< $@

$(IMAGE): packer image/image.pkr.hcl image/install.sh image/cleanup.sh
	rm -rf $(dir $@)
	mkdir -p input-tvm
	./packer build -var="syscores=`nproc`" \
		-var="http_proxy=$$http_proxy" -var="https_proxy=$$https_proxy" \
		image/image.pkr.hcl
	rm -rf input-tvm
	touch $@

packer:
	wget -O packer_$(PACKER_VERSION)_linux_amd64.zip \
	    https://releases.hashicorp.com/packer/$(PACKER_VERSION)/packer_$(PACKER_VERSION)_linux_amd64.zip
	unzip packer_$(PACKER_VERSION)_linux_amd64.zip
	rm -f packer_$(PACKER_VERSION)_linux_amd64.zip

.PHONY: clean
clean:
	rm -rf output-tvm input-tvm packer packer_cache out
	$(MAKE) -C $(VTA_DIR) clean

$(VTA):
	SIMBRICKS_LIB=/simbricks/lib $(MAKE) -C $(VTA_DIR)

.PHONY: experiment_requirements
experiment_requirements: $(VTA) $(IMAGE)
