# Corundum

[Corundum](https://github.com/corundum/corundum) is an open-source, high-performance FPGA-based NIC and platform for in-network compute.

In this example we showcase how one can integrate Corundum (or for that matter similar projects) into SimBricks.
We showcase the integration using [Verilator](https://www.veripool.org/verilator/) to simulate the actual RTL of Corundum.

## Running a Virtual Prototype that uses Corundum

As part of the access to the orchestration demo, you can submit a SimBricks Virtual Prototype for execution 
to our runner, that makes use of the Corundum integration shown in this example.

In order to do so you can simply run the following:

```
simbricks-cli runs submit -f virtual_prototype.py
```

## The SimBricks Integration

The actual integration of Corundum can be found in this example folder. 
If you want to learn more about SimBricks Adapters and on how to integrate simulators into SimBricks, 
check out our [documentation](https://simbricks.readthedocs.io/en/latest/learn/simulator-integration/index.html).

Conceptually the Corundum integration consists of the following pieces:

1) `adapter/corundum_simbricks_adapter.cpp`:

    Within this `corundum_simbricks_adapter.cpp` you can find the actual SimBricks simulator adapter. In this case, the adapter is also the driver of the Corundum simulation. FOr this the Adapter imports the header file representing the top level module of the design under test (DUT) that was created by compiling Corundum using verilator.

2) `orchestration/corundum_orchestration.py`:

    This is a python wrapper around Corundums Adapter. It is essentially an extension to SimBricks orchestration framework and is used to define your virtual prototypes and to submit as well as execute you Virtual Prototypes.

    In this case it consists of a Corundum NIC (`CorundumNIC`) and a Corundum linux host (`CorundumLinuxHost`) system component used to describe the topology of a Virtual Prototype. Additionally the orchestration defines a simulation component (`CorundumVerilatorNICSim`) that represents a simulator choice for the system component. The simulation component does also make use of the compiled cpp adapter mentioned before.

3) `Dockerfile`:

    The Dockerfile is an environment that makes the integration available. It creates an linux image and uses the `Makefile` from this example to compile Corundum using Verilator, to compile the Corundum linux driver, 
    it compiles the cpp Adapter that we mentioned before and makes the python orchestration available.

    When you simply build this Dockerfile, you could simply run it locally to execute the given Virtual Prototype on your machine:

    ```
    $ docker image build --no-cache -t corundum_example_image .
    $ docker run --entrypoint /bin/bash -it --rm --device=/dev/kvm corundum_example_image:latest
    container$ simbricks-run --verbose /corundum_src/virtual_prototype.py
    ```

    When looking closely at the Dockerfile one might notice that it inherits from the `simbricks/simbricks-executor`
    docker image. Therefore, this docker image can be given to SimBricks Runners that can use it. 
    This is a way to make respective Runners the Corundum integration available, such that a virtual prototypes that uses Corundum can be executed in the SimBricks cloud.
    This is also how we made the Corundum integration available in this demo.


## Setup

If you are using the provided devcontainer you are ready to go.

In case you are not, you need to make the python orchestration we created as part of the Corundum integration available.
For this you could e.g. add the current working directory (assuming you are in the same directory as this README) to you `PYTHONPATH`: 

```
export PYTHONPATH=$(pwd)
```