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

import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.e2e_components as e2e


# helper method to create the HostClass
def get_host_class(experiment, host_type):
    HostClass = None
    if host_type == "qemu":
        HostClass = sim.QemuHost
    elif host_type == "qt":

        def qemu_timing(node_config: node.NodeConfig):
            h = sim.QemuHost(node_config)
            h.sync = True
            return h

        HostClass = qemu_timing
    elif host_type == "gt":
        HostClass = sim.Gem5Host
        # e.checkpoint = True
    else:
        raise NameError(host_type)
    return HostClass


# helper method to create NicClass and NodeConfig
# NOTE: the node is important as it provides drivers etc. needed to interface the NIC
def get_nic_class_and_node(nic_type):
    def corundum_linux_node():
        nc = node.CorundumLinuxNode()
        nc.memory = 2048
        return nc

    NicClass = None
    NcClass = None
    if nic_type == "ib":
        NicClass = sim.I40eNIC
        NcClass = node.I40eLinuxNode
    elif nic_type == "cb":
        NicClass = sim.CorundumBMNIC
        NcClass = corundum_linux_node
    elif nic_type == "cv":
        NicClass = sim.CorundumVerilatorNIC
        NcClass = corundum_linux_node
    else:
        raise NameError(nic_type)
    return NicClass, NcClass


def add_host_to_topo_left(topo, host_name, nic, unsynchronized):
    host = e2e.E2ESimbricksHost(host_name)
    if unsynchronized:
        host.sync = e2e.SimbricksSyncMode.SYNC_DISABLED
    host.eth_latency = "1us"
    host.simbricks_component = nic
    topo.add_left_component(host)


def add_host_to_topo_right(topo, host_name, nic, unsynchronized):
    host = e2e.E2ESimbricksHost(host_name)
    if unsynchronized:
        host.sync = e2e.SimbricksSyncMode.SYNC_DISABLED
    host.eth_latency = "1us"
    host.simbricks_component = nic
    topo.add_right_component(host)


def init_ns3_net(net):
    options = {
        "ns3::TcpSocket::SndBufSize": "524288",
        "ns3::TcpSocket::RcvBufSize": "524288",
    }
    net.opt = " ".join([f"--{o[0]}={o[1]}" for o in options.items()])


def init_ns3_dumbbell_topo(topo, link_rate, link_latency):
    # create a dumbbell topology
    topo.data_rate = f"{link_rate}Mbps"
    topo.delay = f"{link_latency}ms"
    topo.queue_size = f"2000000B"
    topo.mtu = f"{1500}"
