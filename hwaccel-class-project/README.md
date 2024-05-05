# Project: Accelerating Applications with Specialized Hardware

## Environment Setup
We have created pre-built docker images for this project, as the environment
setup is complicated and will get a lot worse in later milestones (with ASIC
synthesis tools etc.).

You will need a working docker setup. We recommend doing this natively on Linux,
but Windows should work as well. We have not tested macOS.

### Linux
First off, you will need a working docker installation. How to do this will vary
depending on your distro.  We recommend the server version instead of the
desktop version here (although desktop "should" work too, but we did not test
this). The [docker documentation](https://docs.docker.com/engine/install/) has
instructions for some of them. For more exotic distributions you should consult
your distro documentaiton.

To run the tests you will need a system with kvm virtualization available and
enabled. You only need the kernel support (among other things `/dev/kvm` has to
exist), but no user space qemu-kvm setup etc. is required.

Set `/proc/sys/kernel/perf_event_paranoid` to `1` or less, so that gem5 can
access CPU performance counters.
([Linux Kernel Docs](https://www.kernel.org/doc/Documentation/sysctl/kernel.txt)
have more detail on what this does).

### Windows
You need to install
[docker desktop](https://docs.docker.com/desktop/install/windows-install/).
We have tested with the WSL2 backend. Under Windows you will have to run
`sudo echo 1 >/proc/sys/kernel/perf_event_paranoid` *inside the container*.

### macOS (untested)
*We have not tested OS-X support.*
Here as well you would need at least a working
[docker desktop installation](https://docs.docker.com/desktop/install/mac-install/).

## Using the Environment
The project repository is set up with a [dev container](https://containers.dev/)
configuration file that makes running the container environment relatively easy.

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
yours, mount the worspace directory in the container, and run additional
initialization commands. If you are masochistic enough you can in principle do
this manually as well, but this is at your own risk. ;-)

## Project Milestones
The project milestones are in separate subdirectories `msX`. Per-milestone
instructions can be found in the `README.md` files in those subdirectories.
