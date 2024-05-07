# SimBricks Examples

This repository contains various examples for how to use
[SimBricks](https://github.com/simbricks/simbricks).

The repository is set up with a [dev container](https://containers.dev/)
configuration file that makes running the examples in a container environment
relatively easy. The easiest way to use this is with
[vscode](https://code.visualstudio.com/) and its
[Dev Containers Extension](vscode:extension/ms-vscode-remote.remote-containers).
The [hwaccel-class-project/README.md](hwaccel-class-project/README.md) contains
some instructions on how to set this up.

## Hardware Acceleration Class Project

In this class project you will learn about various aspects of hardware
acceleration, by accelerating an application with specialized hardware. The
projects focus on different aspects of the complete system, such as modifying an
application to take advantage of an accelerator, implementing necessary software
components to interact with the hardware, optimizing the interface between
software and hardware, and even implementing (parts of) a hardware accelerator.
Overall, the main goal is for you to completely understand the full picture of
all system components involved and their interactions. The focus is on the
principles and fundamental constraints rather than specific tools or
applications.

You can find all relevant project files in the `hwaccel-class-project` directory
of this repository. All necessary information can be found in the respective
`README.md` files, starting with
[hwaccel-class-project/README.md](hwaccel-class-project/README.md)

## Memcached Example

This is a small SimBricks experiment using [memcached](https://memcached.org/). 
This requires different images from the ones provided by the included 
[devcontainer](https://github.com/simbricks/simbricks-examples/blob/main/.devcontainer.json).
The purpose of this small example is to show SimBricks users how to build images 
out-of-tree and use them alongside SimBricks.
