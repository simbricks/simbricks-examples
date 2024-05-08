# SimBricks Examples

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](simbricks/simbricks-examples)

This repository contains a series of examples showing how to use
[SimBricks](https://github.com/simbricks/simbricks).

The repository is set up with a [dev container](https://containers.dev/)
configuration that makes running the examples a breeze. For quick tests you
can just
[open the repo in CodeSpaces](https://codespaces.new/simbricks/simbricks-examples).
The easiest way to use this is with
[vscode](https://code.visualstudio.com/) and its
[Dev Containers Extension](vscode:extension/ms-vscode-remote.remote-containers)
([details on setup](#environment-setup))


## Example: Custom Images
Any non-trivial project will typically require you to set up custom disk images
for your simulated hosts to install necessary software dependencies and tools.
In this simple example, we demonstrate an easy way to do this, by creating an
image with [memcached](https://memcached.org/) and a simple client benchmark,
and then configuring and running a simple simulation using this image.
Start [here](custom-image/README.md)

## Example: Hardware Acceleration Class Projects

This is a series of course projects on hardware acceleration, demonstrating a
complete SimBricks-based development flow, from initial software profiling to
Verilog-RTL implementation. The example is split up in multiple project
milestones with step-by-step instructions, for the different development stages.
Start [here](hwaccel-class-project/README.md)


## Environment Setup
This repository is set up with a devcontainer configuration based on the
[SimBricks Docker Images](https://hub.docker.com/u/simbricks). This is the
easiest way of starting to use SimBricks, as you do not have to manually install
dependencies, build SimBricks, or set up other environment components.
You will need a working docker setup. We recommend doing this natively on Linux,
but Windows and OS X should work as well (with some restrictions).

### Linux
First off, you will need a working docker installation. The [docker
documentation](https://docs.docker.com/engine/install/) has instructions. For
some of the simulations (gem5 with kvm-checkpointing) you need a system with kvm
virtualization available and enabled. You only need kernel support (among other
things `/dev/kvm` has to exist), but no user space qemu-kvm setup etc. is
required.

Set `/proc/sys/kernel/perf_event_paranoid` to `1` or less, so that gem5 can
access CPU performance counters.
([Linux Kernel Docs](https://www.kernel.org/doc/Documentation/sysctl/kernel.txt)
have more detail on what this does).

### Windows
You need to install
[docker desktop](https://docs.docker.com/desktop/install/windows-install/).
We have tested with the WSL2 backend. Under Windows you will have to run
`sudo echo 1 >/proc/sys/kernel/perf_event_paranoid` *inside the container*.

### OS X
Here as well you would need at least a working
[docker desktop installation](https://docs.docker.com/desktop/install/mac-install/).
We have not recently tested kvm support on OS-X.

### vscode
The easiest way to use this is with [vscode](https://code.visualstudio.com/) and
its
[Dev Containers Extension](vscode:extension/ms-vscode-remote.remote-containers).
If you have a working docker setup and the extension installed, once
you open a workspace for the cloned repo, vscode will prompt you to **re-open in
the devcontainer**. vscode will then automatically set up the container, start
it, and automatically have the terminals in vscode run inside the container.

### devcontainers CLI
If you prefer using the commandline directly you could give the
[devcontainers cli](https://github.com/devcontainers/cli) a try. This will also
take care of starting and initializing the container, just as with vscode.

You first have to set up the container. For this, run the following command in
the root directory of the project, that is the `hwaccel-class-project` folder
in this repository: `devcontainer up --workspace-folder .`

To then run an interactive shell in the container, run:
`devcontainer exec --workspace-folder . bash`. You can replace bash with any
other commands if you just want to run an individual command.

### Manually running container
Note that devcontainers cli goes quite a bit beyond just `docker run`. Instead
this will set up a user with a correct UID for a user in the container matching
yours, mount the workspace directory in the container, and run additional
initialization commands. But you can do this manually too, if you prefer, in
which case you can directly use the `simbricks/simbricks` docker image.
