# Corundum

[Corundum](https://github.com/corundum/corundum) is an open-source, high-performance FPGA-based NIC and platform for in-network compute.

This folder contains **an example virtual prototype that uses Corundum**, not the integration itself.
The actual SimBricks integration — the Verilator adapter, the orchestration classes and the build of the
`mqnic` Linux driver — lives in its own repository,
[simbricks/component-corundum](https://github.com/simbricks/component-corundum). The python orchestration bits from there are installed as a pip package. That is why this folder only holds a single experiment script.

## What This Example Does

[`virtual_prototype.py`](virtual_prototype.py) describes two Linux hosts, each simulated in QEMU and each
connected to its own Corundum NIC over PCIe. The NICs are simulated at the RTL level with
[Verilator](https://www.veripool.org/verilator/), so the actual Corundum Verilog is executed. Both NICs are
attached to a SimBricks Ethernet switch. The client host pings the server host 20 times and then unloads the
`mqnic` driver; the server just waits.

## Prerequisites

**1. The Corundum simulator must be available to the runner.**

The Corundum pip packages (`simbricks-corundum-sys-py` and `simbricks-corundum-sim-rtl-py`) provide the orchestration classes used in the script. On runner side, users must use the `simbricks-corundum-sim-rtl-bin` conda package to use the `simb_corundum` simulator binary. Our demo Runner has the  `simbricks-corundum-sim-rtl-bin` conda package installed. 

**2. The disk image must contain the `mqnic` driver.**

`CorundumLinuxHost` only *declares* that the guest has to load the `mqnic` kernel module — it does not put
the module into the image. The module has to already be present in the Linux image the hosts boot, which the
script picks with:

```python
distro_disk_image = system.DistroDiskImage(syst, "base")
```

That image is built with [simbricks/image-builder](https://github.com/simbricks/image-builder). Its
`examples/corundum/install-mqnic.sh` stage clones `component-corundum` inside the guest, builds the driver
against the image's kernel and installs it, so appending that stage to an image build is what makes the
image usable here:

```sh
make image EXTRA_SCRIPTS="examples/corundum/install-mqnic.sh"
```

If you run against the SimBricks cloud, the `base` image already ships with the driver installed and there
is nothing to do. If you build images yourself, make sure the image you reference went through that stage —
otherwise the hosts boot fine but fail to bring up the NIC.

## Running the Example

As part of the access to the orchestration demo, you can submit a SimBricks Virtual Prototype for execution
to our runner, that makes use of the Corundum integration.

In order to do so you can simply run the following:

```
simbricks-cli runs submit -f virtual_prototype.py
```

## Where the Integration Lives

This example folder previously contained the whole integration. Everything has moved to
[simbricks/component-corundum](https://github.com/simbricks/component-corundum):

| Used to be here | Now |
|---|---|
| `adapter/corundum_simbricks_adapter.cpp` | `adapter/` in `component-corundum`, built and installed as the `simb_corundum` binary |
| `orchestration/corundum_orchestration.py` | the `simbricks.components.corundum.system` and `simbricks.components.corundum.simulation` packages |
| `Makefile` (verilate Corundum, build the `mqnic` driver) | `component-corundum`'s `Makefile` |
| `Dockerfile` (executor image carrying the integration) | conda packages installed on the runner |

If you want to learn more about SimBricks Adapters and on how to integrate simulators into SimBricks, check
out our [documentation](https://simbricks.readthedocs.io/en/latest/learn/simulator-integration/index.html).
