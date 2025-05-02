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


from simbricks.orchestration import system
from simbricks.orchestration import simulation
from simbricks.orchestration.helpers import instantiation as inst_helpers
from simbricks.utils import base as utils_base


"""
This list is used and expected
"""
instantiations = []


"""
PARAMETERS
"""
sys_nic = system.CorundumNIC
sys_host = system.CorundumLinuxHost

sim_nic = simulation.CorundumBMNICSim
sim_host = simulation.QemuSim

synchronized = False


"""
System Specification
"""
syst = system.System()

# create disk images
distro_disk_image = system.DistroDiskImage(syst, "base")

# create client
host0 = sys_host(syst)
host0.add_disk(distro_disk_image)
host0.add_disk(system.LinuxConfigDiskImage(syst, host0))
# create client NIC
nic0 = sys_nic(syst)
nic0.add_ipv4("10.0.0.1")
host0.connect_pcie_dev(nic0)

# create server
host1 = sys_host(syst)
host1.add_disk(distro_disk_image)
host1.add_disk(system.LinuxConfigDiskImage(syst, host1))
# create server NIC
nic1 = sys_nic(syst)
nic1.add_ipv4("10.0.0.2")
host1.connect_pcie_dev(nic1)

# set client application
client_app = system.NetperfClient(h=host0, server_ip=nic1._ip)
client_app.wait = True
host0.add_app(client_app)
# set server application
server_app = system.NetperfServer(h=host1)
host1.add_app(server_app)

# create switch and connect NICs to switch
switch = system.EthSwitch(syst)
switch.connect_eth_peer_if(nic0._eth_if)
switch.connect_eth_peer_if(nic1._eth_if)


"""
Simulator Choice
"""
sim = simulation.Simulation(name="Milestone-1-simulation", system=syst)

host_inst0 = sim_host(sim)
host_inst0.add(host0)

host_inst1 = sim_host(sim)
host_inst1.add(host1)

nic_inst0 = sim_nic(simulation=sim)
nic_inst0.add(nic0)

nic_inst1 = sim_nic(simulation=sim)
nic_inst1.add(nic1)

net_inst = simulation.SwitchNet(sim)
net_inst.add(switch)

if synchronized:
    sim.enable_synchronization(amount=500, ratio=utils_base.Time.Nanoseconds)


"""
Instantiation
"""
instance = inst_helpers.simple_instantiation(sim)

instantiations.append(instance)
