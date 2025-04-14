# SimBricks Examples

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/simbricks/simbricks-examples/?quickstart=1)

This repository contains a series of examples showing how to use
[SimBricks](https://github.com/simbricks/simbricks).

## Getting Started with the Examples in This Repository 

Welcome! To use the examples shown in this repository effectively, you’ll need to follow a few setup steps. These steps ensure that you have access to the SimBricks cloud backend and the necessary tools to run the examples provided.

### Register for the SimBricks Demo

The examples in this repository utilize the SimBricks cloud version. To run them as-is, you’ll need access to this backend. Start by [registering for a SimBricks demo account](https://www.simbricks.io/demo/). The registration ensures you have the proper credentials to interact with the backend.

### Clone This Repository and Set Up a Python Virtual Environment

Once you have your SimBricks demo account, clone this repository and set up a Python virtual environment to manage dependencies. Here’s how you can do that:

1. Clone the repository:
   ```
   git clone git@github.com:simbricks/simbricks-examples.git
   cd simbricks-examples
   ```
2. Create and activate a [Python virtual environment](https://docs.python.org/3/tutorial/venv.html):
   ```
   python3 -m venv venv
   source venv/bin/activate
   ```

### Adjust the Setup Script and Set Up Environment Variables

After creating your SimBricks demo account, you’ll need to adjust the setup.sh file located in the top-level directory of this repository:
- Open the setup.py file.
- Locate the line that sets the NAMESPACE variable.
- Replace <Your Demo email address> with the email address you used to create your demo account.

Once you’ve updated the setup.py file, run the setup.sh script to set the necessary environment variables:
```
source simbricks-client.env
```
This step ensures your environment is correctly configured to interact with SimBricks.

### Install Required SimBricks Python Packages

To run the examples in this repository, you need to install the following SimBricks Python packages:
- simbricks-orchestration: For creating virtual prototype configurations as shown in the examples.
- simbricks-client: For sending configurations to the SimBricks server via Python.
- simbricks-cli: For managing configurations via the terminal CLI.

You can install these packages using pip:
```
pip install -r requirements.txt
```

With the above steps completed, you’re ready to dive into the examples provided in this repository.

**If you encounter any issues, consult the SimBricks [documentation](https://simbricks.readthedocs.io/en/latest/) or reach out to us directly.** 

<!-- The repository is set up with a [dev container](https://containers.dev/)
configuration that makes running the examples a breeze. For quick tests you
can just
[open the repo in CodeSpaces](https://codespaces.new/simbricks/simbricks-examples).
The easiest way to use this is with
[vscode](https://code.visualstudio.com/) and its
[Dev Containers Extension](vscode:extension/ms-vscode-remote.remote-containers)
([details on setup](#environment-setup)) -->

## First Steps
Under [`first-steps/first_steps.ipynb`](first-steps/first_steps.ipynb) you will
find an iPython Notebook with an interactive introductions for your first steps
with SimBricks. This introduces the basic concepts for how to configure and run
SimBricks virtual prototypes. For initial quick playing around, you can even
[immediately open this in the browser with
mybinder](https://mybinder.org/v2/gh/simbricks/labs/main?urlpath=git-pull%3Frepo%3Dhttps%253A%252F%252Fgithub.com%252Fsimbricks%252Fsimbricks-examples%26urlpath%3Dlab%252Ftree%252Fsimbricks-examples%252Ffirst-steps%252Ffirst_steps.ipynb%26branch%3Dmain)
(although there you will not have enough compute for anything interesting).

## Networking Case Study

Within the `netwroking-case-study` folder you will find SimBricks examples that demonstrates how to leverage SimBricks orchestration framework to create increasingly complex virtual prototypes of network topologies.
It makes use of the concepts shown in the FirstSteps example and shows a simple setup in which multiple components of a virtual prototype are simulated by the same simulator. 

<!-- ## Example: Custom Images
Any non-trivial project will typically require you to set up custom disk images
for your simulated hosts to install necessary software dependencies and tools.
In this simple example, we demonstrate an easy way to do this, by creating an
image with [memcached](https://memcached.org/) and a simple client benchmark,
and then configuring and running a simple simulation using this image.
Start [here](custom-image/README.md) -->

<!-- ## Example: Hardware Acceleration Class Projects

This is a series of course projects on hardware acceleration, demonstrating a
complete SimBricks-based development flow, from initial software profiling to
Verilog-RTL implementation. The example is split up in multiple project
milestones with step-by-step instructions, for the different development stages.
Start [here](hwaccel-class-project/README.md) -->

<!-- ## Example: Hardware Acceleration JPEG Decoder

You can plug your own RTL into SimBricks for end-to-end evaluation or for
full-system integration testing instead of writing complex Verilog testbenches.
Here, we integrate an [open-source Verilog JPEG
decoder](https://github.com/ultraembedded/core_jpeg_decoder) as a PCIe-based HW
accelerator along with the necessary user-space driver. -->


<!-- ## Environment Setup
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


### Windows
You need to install
[docker desktop](https://docs.docker.com/desktop/install/windows-install/).
We have tested with the WSL2 backend.

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
which case you can directly use the
[`simbricks/simbricks-examples` docker image](https://hub.docker.com/r/simbricks/simbricks-examples). This image just extends the `simbricks/simbricks` image
some development tools (including ipython). -->

<!-- ###
Finally, you can of course also build everything locally. For that please refer
to the [README in the main SimBricks repo](https://github.com/simbricks/simbricks/blob/main/README.md). -->