# End-to-End Evaluation of Verilog JPEG Decoder

In this subdirectory, we integrate an [open-source Verilog JPEG
decoder](https://github.com/ultraembedded/core_jpeg_decoder) into SimBricks. We
connect it as a HW accelerator via PCIe to a Linux machine and measure the time
it takes (from the point of view of the application) to decode individual JPEG
images.

Additionally, this subdirectory can, with a few minor modifications, serve as an
integration test for your RTL design, where you replace complex Verilog
testbenches by directly plugging your RTL into a full-system SimBricks testbed
and checking the output in the produced JSON file with Python testcases.


## System Details
The RTL is simulated by Verilator and the necessary adapters to integrate with
Simbricks are implemented in
[jpeg_decoder_verilator.cc](./jpeg_decoder_verilator.cc), which is also the main
entry point. On the machine-side, we invoke a user-space PCIe driver
([pci_driver.cc](./pci_driver.cc)), which pokes the JPEG decoder's registers to
write the DMA addresses for source and destination, send a start signal, and
check the status. The JPEG decoder then uses DMA to read the source image and
write the decoded data back.

## Building

Before you can run the experiment, you need to build the `pci_driver` and
`jpeg_decoder_verilator`. For this, run the following:
```shell
$ git submodule update --init --recursive
$ make
```

## Running the Experiment

The SimBricks experiment is defined in
[jpeg_decoder_exp.py](./jpeg_decoder_exp.py) along with two variants. The first
uses QEMU with KVM for a purely functional simulation, which is fast but
measurements are also meaningless. You can run it via `make run-qemu`.

The second variant uses gem5 with the `TimingSimpleCPU` for a timing-accurate
simulation, where Verilator and gem5 synchronize through the SimBricks protocol.
Since gem5 is considerably slower in this accurate mode, so is the overall
full-system simulation. You can run it with `make run-gem5`.

By default, the experiment produces debug information by dumping the decoded
images as base64 to standard out, as well as producing a waveform `.vcd` file.
You can tweak this behavior in [jpeg_decoder_exp.py](./jpeg_decoder_exp.py), as
well as which images you want to decode.

There are three example images under [test_imgs/](./test_imgs/), along with a
Python script to re-encode your own images with the correct JPEG settings that
the decoder expects.

## Evaluating the Experiment

Finally, to automatically process the experiment's output, run `make
process-out-qemu` or `make process-out-gem5` depending on which experiment
variant you ran. This will print the end-to-end image decoding measurements to
the terminal and render the base-64 encoded images so you can visually compare
them to the original. Note that there's a loss in color precision since the JPEG
decoder outputs RGB565 (5 bits red, 6 bits green, 5 bits blue) instead of
RGB888.

If rendering the images from within the devcontainer fails with the message
`Authorization required, but no authorization protocol specified`, then run
`xhost local:root` on your host.

## Architectural Parameters

There are some architectural parameters that you can tweak. The first one is the
PCIe latency between machine and accelerator, which can be adjusted in
[jpeg_decoder_exp.py](./jpeg_decoder_exp.py). Further, in
[include/jpeg_decoder_verilator.hh](include/jpeg_decoder_verilator.hh), you can
tweak the number of concurrently pending AXI requests that the JPEG decoder can
issue on the AXI subordinate (slave) interface at any time. The current number
captures the bahavior of the PCIe IP block on our FPGA.

Finally, you can enable support for decoding JPEG images that use optimized
Huffman tables by setting `SUPPORT_WRITABLE_DHT=1` in
[rtl/src_v/jpeg_decoder.v](rtl/src_v/jpeg_decoder.v). This substantially
increases the size and reduces performance.