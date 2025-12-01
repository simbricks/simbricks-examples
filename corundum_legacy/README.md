# Corundum

> [!NOTE]
> This example does use a legacy version of Corundum. For a more up-to-date Corundum RTL integration check out the other Corundum example within this repository

[Corundum](https://github.com/corundum/corundum) is an open-source, high-performance FPGA-based NIC and platform for in-network compute.

In this example we showcase how one can integrate Corundum (or for that matter similar projects) into SimBricks.
We showcase the integration using [Verilator](https://www.veripool.org/verilator/) to simulate the actual RTL of Corundum and provide a behavioral model that can be used in place of the RTL version for functional tests.

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

1) `behavioral_model`:

    Within the `behavioral_model` you can find the Corundum behavioral model used as part of this example. When looking closely at the integration of the behavioral model, one notices the following line `runner = new nicbm::Runner(dev);` in the behavioral models main function. The `nicbm::Runner` is a helper class that ships with SimBricks core lib that can make it easier to integrate NIC models in certain cases into SimBricks. The code for the `Runner` can be found in our [core repository](https://github.com/simbricks/simbricks/blob/d2c55ca2bb9d06baacbf0c8c39b1e15fb4c75ce4/lib/simbricks/nicbm/nicbm.cc#L646C1-L757C2). 

2) `rtl_model`:

    Within the `rtl_model` you can find the actual SimBricks adapter for the Corundum RTL integration. In this case, the adapter is the driver of the Corundum simulation. For this the Adapter imports the header file representing the top level module of the design under test (DUT) that was created by compiling Corundum using verilator. A good starting point for understanding the adapter is the [main function](https://github.com/simbricks/simbricks-examples/blob/b64afa5d5f35332c2817722a24570d0f6739c9c1/corundum_legacy/rtl_model/corundum_verilator.cc#L888C1-L1142C2) driving the verilated model execution.

3) `mqnic_driver`:

    The `mqnic_driver` folder contains the unmodified Corundum linux driver for the Corundum NIC we use in the simulated hosts to connect to the simulated Corundum NICs. 

4) `orchestration_corundum_legacy/corundum_orchestration.py`:

    This is a python wrapper around Corundums Adapter. It is essentially an extension to SimBricks orchestration framework and is used to define your virtual prototypes and to submit as well as execute you Virtual Prototypes.

    In this case it consists of a Corundum NIC (`CorundumNIC`) and a Corundum linux host (`CorundumLinuxHost`) system component used to describe the topology of a Virtual Prototype. Additionally the orchestration defines two simulation components (`CorundumVerilatorNICSim`, `CorundumBMNICSim`) that represent the simulator choice that can be made for the system component. These simulation components do also make use of the respective compiled cpp adapters mentioned before.

5) `virtual_prototype.py`:

    This is a simple virtual prototype that creates a virtual prototype consisting of a two simulated hosts, each connected to a Corundum NIC simulation which are in turn connected through a simple switch simulation. This virtual prototype can be simulated as shown above.

6) `Dockerfile`:

    The Dockerfile is an environment that makes the integration available. It creates an linux image and uses the `Makefile` from this example to compile Corundum using Verilator, to compile the Corundum linux driver, 
    it compiles the cpp Adapter that we mentioned before and makes the python orchestration available.

    When you simply build this Dockerfile, you could simply run it locally to execute the given Virtual Prototype on your machine:

    ```
    $ docker image build --no-cache -t corundum_legacy .
    $ docker run --entrypoint /bin/bash -it --rm --device=/dev/kvm corundum_legacy:latest
    container$ simbricks-run --verbose virtual_prototype.py
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