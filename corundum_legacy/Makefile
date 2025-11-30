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

simbricks_base := /simbricks
lib_dir := $(simbricks_base)/lib
simbricks_lib_dir := $(lib_dir)/simbricks
lib_nicif := $(simbricks_lib_dir)/nicif/libnicif.a
lib_netif := $(simbricks_lib_dir)/network/libnetwork.a
lib_pcie := $(simbricks_lib_dir)/pcie/libpcie.a 
lib_base := $(simbricks_lib_dir)/base/libbase.a
lib_parser := $(simbricks_lib_dir)/parser/libparser.a
lib_nicbm := $(simbricks_lib_dir)/nicbm/libnicbm.a

mqnic_dir := ./mqnic_driver
kernel_dir := $(wildcard $(simbricks_base)/images/kernel/linux-*/)

dir_corundum_bm := ./behavioral_model
bin_corundum_bm := $(dir_corundum_bm)/corundum_bm

CLEAN := $(bin_corundum_bm)

.PHONY: all behavioral_model rtl_model mqnic_driver clean

all: behavioral_model rtl_model mqnic_driver
.DEFAULT_GOAL := all


$(bin_corundum_bm): $(dir_corundum_bm)/corundum_bm.cpp $(dir_corundum_bm)/corundum_bm.hpp
	$(CXX) -Wall -O2 -I$(abspath $(lib_dir)) -Ibehavioral_model -iquote $(simbricks_base) -o $@ $(dir_corundum_bm)/corundum_bm.cpp $(lib_nicbm) $(lib_nicif) $(lib_base) $(lib_pcie) $(lib_netif)

behavioral_model: $(bin_corundum_bm)

include rtl_model/Makefile

mqnic_driver:
	$(MAKE) -C $(kernel_dir) M=$(abspath $(mqnic_dir)) modules

# Clean everything
clean:
	rm -rf $(CLEAN)
	$(MAKE) -C $(kernel_dir) M=$(abspath $(mqnic_dir)) clean

