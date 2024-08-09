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

CFLAGS := -I/simbricks/lib -O3 -Wall -Wextra
JPGD_SRCS := rtl/src_v rtl/jpeg_core/src_v
JPGD_TOP := rtl/src_v/jpeg_decoder.v
LIB_SIMBRICKS := /simbricks/lib/libsimbricks.a

all: pci_driver jpeg_decoder_verilator

.PHONY: run-qemu
run-qemu: pci_driver jpeg_decoder_verilator
	simbricks-run --verbose --force --filter=jpeg_decoder-qemu jpeg_decoder_exp.py

.PHONY: run-gem5
run-gem5: pci_driver jpeg_decoder_verilator
	simbricks-run --verbose --force --filter=jpeg_decoder-gem5 jpeg_decoder_exp.py

.PHONY: process-out-qemu
process-out-qemu:
	python process_exp_output.py out/jpeg_decoder-qemu-1.json


.PHONY: process-out-gem5
process-out-gem5:
	python process_exp_output.py out/jpeg_decoder-gem5-1.json

pci_driver: pci_driver.cc
	g++ -o $@ -static $(CFLAGS) $<

jpeg_decoder_verilator: jpeg_decoder_verilator.cc
	verilator --cc -O3 \
		--trace --no-trace-params --exe \
		-Wno-WIDTH \
		-CFLAGS "$(CFLAGS)" \
		--Mdir obj_dir \
		$(addprefix -y ,$(JPGD_SRCS)) \
		$(JPGD_TOP) \
		$(LIB_SIMBRICKS) \
		$<
	make -C obj_dir -f Vjpeg_decoder.mk
	cp obj_dir/Vjpeg_decoder $@

.PHONY: clean
clean:
	rm -rf obj_dir jpeg_decoder_verilator pci_driver
