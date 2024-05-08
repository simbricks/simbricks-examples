# Custom Image Example (Memcached)

This example shows how to build a custom host disk image to install additional
packages to use in simulated SimBricks hosts. We start with the SimBricks base
disk, built on the official Ubuntu 22.04 minimal cloud image, extended with some
standard tools, kernel headers, etc. (see the [build
script](https://github.com/simbricks/simbricks/blob/main/images/scripts/install-base.sh)
for the base image). We build a custom disk image to also install `memcached`
and the `memaslap` client, and then set up a simple simulation configuration
with a client and server machine, corresponding NICs and a switch.

## TL;DR:
```
# quick interactive run with qemu
$ make run-qemu

# slower performance evaluation with gem5
$ make run-gem5
```

## Building the image
A simple way to create disk images automaticaly is with
[packer](https://www.packer.io/). Packer requires a configuration file to
coordinate the build process (see [`image/image.pkr.hcl`](image/image.pkr.hcl)).
From there, you can load additional files into the image, and run commands. We
start with the SimBricks base image (`/simbricks/images/output-base/base` in the
container). We then loads and executes two shell scripts
([`image/install.sh`](image/install.sh) and
[`image/cleanup.sh`](image/cleanup.sh)). The install script, installs memcache
with the ubuntu package manager, and then builds the client from source. The
cleanup script removes temporary files etc. for a more compact disk image.

The `Makefile` contains targets to build the image in qcow-format (compact, and
directly usable by qemu) under `output-memcached/memcached`, and also to convert
it to the raw format for gem5 (`output-memcached/memcached.raw`).

## Simulation Configuration
[`memcached.py`](memcached.py) is an expample SimBricks simulation script that
generates two different simulation configurations. Both comprise two hosts, two
Intel 40G NICs, and a basic switch. One configuration is a fast functional
simulation `memcached-qemu`, and the other one uses gem5 for a more detailed
performance simulation `memcached-gem5`. The SimBricks
[documentation](https://simbricks.readthedocs.io/en/latest/user/orchestration.html)
contains more documentation on how the configuration scripts work.

The key component in this exapmle is configuring the hosts to use the custom
disk image. For this, we set the `disk_image` property of the involved host
simulator's *node config* to the **absolute** image path.

You can then either run the simulations manually with `simbricks-run` or use the
`run-qemu` and `run-gem5` make targets that will also ensure the image is built
first.