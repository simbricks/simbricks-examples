# Copyright 2021 Max Planck Institute for Software Systems, and
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

import re
import asyncio

from simbricks.orchestration import system
from simbricks.orchestration import simulation
from simbricks.orchestration import instantiation
from simbricks.utils import base as utils_base
from simbricks.client.opus import base as opus_base


"""
This list is used and expected
"""
instantiations = []


"""
PARAMETERS
"""
synchronized = False


"""
System Specification
"""
syst = system.System()

# create client
host0 = system.I40ELinuxHost(syst)
host0.add_disk(system.DistroDiskImage(h=host0, name="base"))
host0.add_disk(system.LinuxConfigDiskImage(h=host0))
# create client NIC
nic0 = system.IntelI40eNIC(syst)
nic0.add_ipv4("10.0.0.1")
host0.connect_pcie_dev(nic0)

# create server
host1 = system.I40ELinuxHost(syst)
host1.add_disk(system.DistroDiskImage(h=host1, name="base"))
host1.add_disk(system.LinuxConfigDiskImage(h=host1))
# create server NIC
nic1 = system.IntelI40eNIC(syst)
nic1.add_ipv4("10.0.0.2")
host1.connect_pcie_dev(nic1)

# set client application
client_app = system.IperfTCPClient(h=host0, server_ip=nic1._ip)
client_app.wait = True
host0.add_app(client_app)
# set server application
server_app = system.IperfTCPServer(h=host1)
host1.add_app(server_app)

# create switch and connect NICs to switch
switch = system.EthSwitch(syst)
switch.connect_eth_peer_if(nic0._eth_if)
switch.connect_eth_peer_if(nic1._eth_if)


"""
Simulator Choice
"""
sim = simulation.Simulation(name="My-simple-simulation", system=syst)

host_inst0 = simulation.QemuSim(sim)
host_inst0.add(host0)

host_inst1 = simulation.QemuSim(sim)
host_inst1.add(host1)

nic_inst0 = simulation.I40eNicSim(simulation=sim)
nic_inst0.add(nic0)

nic_inst1 = simulation.I40eNicSim(simulation=sim)
nic_inst1.add(nic1)

net_inst = simulation.SwitchNet(sim)
net_inst.add(switch)


# if synchronized set, enable synchronization for all SimBricks channels
if synchronized:
    sim.enable_synchronization(amount=500, ratio=utils_base.Time.Nanoseconds)


"""
Instantiation
"""
instance = instantiation.Instantiation(sim)
# Create a single runtime Fragement that is supposed to contain all simulators for execution.
# In case you plan to distribute the execution of your virtual prototype across multiple machines,
# you would have to define multiple such Fragments.
fragment = instantiation.Fragment()
fragment.add_simulators(*sim.all_simulators())
instance.fragments = [fragment]
instantiations.append(instance)


"""
This is how you can use the SimBricks Api from python instead of the CLI. This 
can e.g. be useful to immidiately parse an experiments output in python. 
"""
if __name__ == "__main__":

    # create and send simulation run to the SimBricks backend
    run_id = asyncio.run(opus_base.create_run(instance))

    # helper function to create and parse the experiment output
    async def iperf_throughput() -> None:

        # Regex to match output lines from iperf client
        tp_pat = re.compile(
            r"\[ *\d*\] *([0-9\.]*)- *([0-9\.]*) sec.*Bytes *([0-9\.]*) ([GM])bits.*"
        )
        throughputs = []
        # iterate through host output
        line_gen = opus_base.ConsoleLineGenerator(run_id=run_id, follow=True)
        async for _, line in line_gen.generate_lines():
            m = tp_pat.match(line)
            if not m:
                continue
            if m.group(4) == "G":
                throughputs.append(float(m.group(3)) * 1000)
            elif m.group(4) == "M":
                throughputs.append(float(m.group(3)))

        avg_throughput = sum(throughputs) / len(throughputs)
        print(f"Iperf Throughput : {avg_throughput} Mbps")

    asyncio.run(iperf_throughput())
