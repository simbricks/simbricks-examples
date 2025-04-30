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


dir_corundum := ./corundum
verilator_dir_corundum := $(dir_corundum)/obj_dir
verilog_interface_name := mqnic_core_axi
verilator_interface_name := V$(verilog_interface_name)
verilator_src_corundum := $(verilator_dir_corundum)/$(verilator_interface_name).cpp
verilator_bin_corundum := $(verilator_dir_corundum)/$(verilator_interface_name)
adapter_main := adapter/corundum_simbricks_adapter
corundum_simbricks_adapter_src := $(adapter_main).cpp
corundum_simbricks_adapter_bin := $(adapter_main)

simbricks_base := /simbricks

mqnic_dir := $(dir_corundum)/modules/mqnic
kernel_dir := $(wildcard $(simbricks_base)/images/kernel/linux-*/)

lib_dir := $(simbricks_base)/lib
simbricks_lib_dir := $(lib_dir)/simbricks
lib_nicif := $(simbricks_lib_dir)/nicif/libnicif.a
lib_netif := $(simbricks_lib_dir)/network/libnetwork.a
lib_pcie := $(simbricks_lib_dir)/pcie/libpcie.a 
lib_base := $(simbricks_lib_dir)/base/libbase.a
lib_parser := $(simbricks_lib_dir)/parser/libparser.a

VERILATOR = verilator
VFLAGS = -Wno-WIDTH -Wno-PINMISSING -Wno-LITENDIAN -Wno-IMPLICIT -Wno-SELRANGE \
    -Wno-CASEINCOMPLETE -Wno-UNSIGNED -Wno-UNOPTFLAT --timescale 1ns/1ps

TOPLEVEL = mqnic_core_axi


$(verilator_src_corundum):
	$(VERILATOR) $(VFLAGS) --cc -O3 \
	    -CFLAGS "-I$(abspath $(lib_dir)) -iquote $(simbricks_base) -O3 -g -Wall -Wno-maybe-uninitialized" \
	    --Mdir $(verilator_dir_corundum) \
		--top-module $(verilog_interface_name) \
		--trace \
	    -y $(dir_corundum)/fpga/common/rtl \
		-y $(dir_corundum)/fpga/common/lib/axis/rtl \
		-y $(dir_corundum)/fpga/common/lib/eth/rtl \
		-y $(dir_corundum)/fpga/common/lib/pcie/rtl \
	    -y $(dir_corundum)/fpga/lib/axi/rtl \
	    -y $(dir_corundum)/fpga/lib/eth/lib/axis/rtl/ \
	    -y $(dir_corundum)/fpga/lib/pcie/rtl \
	    $(dir_corundum)/fpga/common/rtl/$(verilog_interface_name).v \
	    $(dir_corundum)/fpga/common/rtl/mqnic_tx_scheduler_block_rr.v \
		--exe $(abspath $(corundum_simbricks_adapter_src)) $(abspath $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base) $(lib_parser))

$(verilator_bin_corundum): $(verilator_src_corundum) $(corundum_simbricks_adapter_src)
	$(MAKE) -C $(verilator_dir_corundum) -f $(verilator_interface_name).mk

$(corundum_simbricks_adapter_bin): $(verilator_bin_corundum)
	cp $< $@

adapter: $(corundum_simbricks_adapter_bin)

driver:
	@echo $(kernel_dir)
	$(MAKE) -C $(kernel_dir) M=$(abspath $(mqnic_dir)) modules

all: driver adapter
.DEFAULT_GOAL := all

clean: 
	rm -rf $(corundum_simbricks_adapter_bin) $(verilator_dir_corundum) $(OBJS) ready

.PHONY: all driver adapter clean