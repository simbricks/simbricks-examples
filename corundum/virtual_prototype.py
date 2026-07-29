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
from simbricks.components.qemu import simulation as qemu_sim
from simbricks.components.net.simulation import base as net_sim
from simbricks.orchestration.helpers import simulation as sim_helpers
from simbricks.orchestration.helpers import instantiation as inst_helpers

"""
Import the orchestration bits of the Corundum integration. These are provided by the
simbricks-corundum-sys-py and simbricks-corundum-sim-rtl-py conda packages, which are built
from https://github.com/simbricks/component-corundum.
"""
from simbricks.components.corundum import system as corundum_sys
from simbricks.components.corundum import simulation as corundum_sim


"""
This list is used and expected
"""
instantiations = []


"""
System Specification
"""
syst = system.System("Corundum-Example")

# Create disk images. Note that CorundumLinuxHost below only declares that the guest must load
# the mqnic driver, it does not provide the driver itself. The image referenced here must
# therefore already contain the mqnic kernel module. Such an image is built with
# https://github.com/simbricks/image-builder by including its install-mqnic.sh stage.
distro_disk_image = system.DistroDiskImage(syst, "base")

# create client
host0 = corundum_sys.CorundumLinuxHost(syst)
host0.name = "client-Host"
host0.add_disk(distro_disk_image)
host0.add_disk(system.LinuxConfigDiskImage(syst, host0))
# create client NIC
nic0 = corundum_sys.CorundumNIC(syst)
nic0.name = "client-NIC"
nic0.add_ipv4("10.0.0.1")
host0.connect_pcie_dev(nic0)

# create server
host1 = corundum_sys.CorundumLinuxHost(syst)
host1.name = "server-Host"
host1.add_disk(distro_disk_image)
host1.add_disk(system.LinuxConfigDiskImage(syst, host1))
# create server NIC
nic1 = corundum_sys.CorundumNIC(syst)
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
        system.FullSystemHost: qemu_sim.QemuSim,
        corundum_sys.CorundumNIC: corundum_sim.CorundumVerilatorNICSim,
        system.EthSwitch: net_sim.SwitchNet,
    },
)


"""
Instantiation
"""
instance = inst_helpers.simple_instantiation(sim)
# Here we ensure that the fragment we created is executed by a runner that has the Corundum
# simulator available, i.e. one on which the Corundum conda packages are installed.
fragment = instance.fragments[0]
fragment.fragment_executor_tag = "corundum_executor"

instantiations.append(instance)
