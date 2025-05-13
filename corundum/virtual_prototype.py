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
from simbricks.orchestration.helpers import simulation as sim_helpers
from simbricks.orchestration.helpers import instantiation as inst_helpers

"""
Import the orchestration bits we created as part of the Corundum integration.
Note that we must use the name of the package as present within the Docker Container
used by the Runner.
"""
from orchestration import corundum_orchestration as co


"""
This list is used and expected
"""
instantiations = []


"""
System Specification
"""
syst = system.System()

# create disk images
distro_disk_image = system.DistroDiskImage(syst, "base")

# create client
host0 = co.CorundumLinuxHost(syst)
host0.name = "client-Host"
host0.add_disk(distro_disk_image)
host0.add_disk(system.LinuxConfigDiskImage(syst, host0))
# create client NIC
nic0 = co.CorundumNIC(syst)
nic0.name = "client-NIC"
nic0.add_ipv4("10.0.0.1")
host0.connect_pcie_dev(nic0)

# create server
host1 = co.CorundumLinuxHost(syst)
host1.name = "server-Host"
host1.add_disk(distro_disk_image)
host1.add_disk(system.LinuxConfigDiskImage(syst, host1))
# create server NIC
nic1 = co.CorundumNIC(syst)
nic1.name = "server-NIC"
nic1.add_ipv4("10.0.0.2")
host1.connect_pcie_dev(nic1)

# set client application
client_app = system.GenericRawCommandApplication(
    host0,
    [
        "mount proc /proc -t proc",
        "mount -t sysfs sysfs /sys",
        "sleep 2",
        f"ping -c 20 {nic1._ip}",
        "rmmod -v mqnic",
    ],
)
client_app.wait = True
host0.add_app(client_app)
# set server application
server_app = system.Sleep(host1, infinite=True)
host1.add_app(server_app)

# create switch and connect NICs to switch
switch = system.EthSwitch(syst)
switch.connect_eth_peer_if(nic0._eth_if)
switch.connect_eth_peer_if(nic1._eth_if)


"""
Simulator Choice
"""
sim = sim_helpers.simple_simulation(
    syst,
    compmap={
        system.FullSystemHost: simulation.QemuSim,
        co.CorundumNIC: co.CorundumVerilatorNICSim,
        system.EthSwitch: simulation.SwitchNet,
    },
)


"""
Instantiation
"""
instance = inst_helpers.simple_instantiation(sim)
# Here we ensure that the runner does choose a proper docker image (the image defined in this repository)
# for executing the fragment we created.
fragment = instance.fragments[0]
fragment._fragment_executor_tag = "corundum_executor"

instantiations.append(instance)
