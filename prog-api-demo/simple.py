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

from simbricks.orchestration import simulation as sim
from simbricks.orchestration import system
from simbricks.orchestration.helpers import instantiation as inst_helpers
from simbricks.orchestration.helpers import simulation as sim_helpers
from helpers import sys_host_nic

"""
Simple example of a simulation: All components are executed in the same fragment.
 _________________________________________________________________________
|                                                                        |
|  Iperf-Server -- Server-NIC -- Switch ----- Client-NIC -- Iperf-CLient |
|________________________________________________________________________|
"""


"""
System configuration
"""
sys = system.System()

# We create a Linux disk image instance that will be used by the hosts we create.
# The image used here is provided by SimBricks, user can however also provide custom images.
distro_disk_image = system.DistroDiskImage(sys, "base")

# Configure the server to start an Iperf server by adding an application to the server object.
server_host, server_nic = sys_host_nic(
    sys, distro_disk_image, "10.0.0.1", "Iperf-Server", "Server-NIC"
)
server_host.add_app(system.IperfTCPServer(server_host))

# Configure the client to start an Iperf client by adding an application to the client object.
# Besides, we set the wait flag on the application to tell SimBricks to run until this application is completed.
client_host, client_nic = sys_host_nic(
    sys, distro_disk_image, "10.0.0.2", "Iperf-Client", "Client-NIC"
)
ping_client_app = system.IperfTCPClient(client_host, server_nic._ip)
ping_client_app.wait = True
client_host.add_app(ping_client_app)

# Create a network switch that connects the server and client NICs with each other.
switch0 = system.EthSwitch(sys)
for nic in [server_nic, client_nic]:
    switch0.connect_eth_peer_if(nic._eth_if)


"""
Simulation configuration
"""

# We make a simulator choice by simply mapping component types to simulators.
simulation = sim_helpers.simple_simulation(
    sys,
    compmap={
        system.FullSystemHost: sim.QemuSim,
        system.IntelI40eNIC: sim.I40eNicSim,
        system.EthSwitch: sim.SwitchNet,
    },
)
# Optionally enable synchronization
# simulation.enable_synchronization()


"""
Instantiation configuration
"""

# Instantiate the virtual prototype
instantiation = inst_helpers.simple_instantiation(simulation)


# We define a list of 'Instantiation' objects, with the name 'instantiations'.
# This list is used by SimBricks to create runs and execute simulations.
# Every SimBricks VP script must define this list.
instantiations = [instantiation]
