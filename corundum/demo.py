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

import simbricks.orchestration.e2e_components as e2e
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.simulator_utils import create_basic_hosts

from functools import reduce

# needed for dot graph
import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
from visualize import experiment_graph_e2e_dumbbell
from helpers import (
    add_host_to_topo_left,
    add_host_to_topo_right,
    get_host_class,
    get_nic_class_and_node,
    init_ns3_net,
    init_ns3_dumbbell_topo,
)


# parameters for eperiment
link_rate = 200  # in Mbps
link_latency = 5  # in ms
bdp = int(link_rate * link_latency / 1000 * 10**6)  # Bandwidth-delay product
ip_start = "192.168.64.1"
num_ns3_hosts = 0
nic_type = "cv"  # one of: 'cv', 'cb'
# (amount, host_type), host_type one of qt, gt, qemu,
hos_conf = [(1, "qt"), (1, "gt")]
total_simbricks_hosts = reduce(lambda prev, cur: prev + cur[0], hos_conf, 0)

# sanity check and check sync
unsynchronized = False
qemu_hosts = list(filter(lambda e: e[1] == "qemu", hos_conf))
if len(qemu_hosts) > 0 and len(qemu_hosts) != len(hos_conf):
    raise Exception("cannot mix synchronized with unsynchronized hosts")
elif len(qemu_hosts) > 0:
    unsynchronized = True

# create the actual experiment
experiments = []

net = sim.NS3E2ENet()
init_ns3_net(net)

# create a dumbbell topology
topology = E2EDumbbellTopology()
init_ns3_dumbbell_topo(topology, link_rate, link_latency)
net.add_component(topology)

# create ns3 applications to create background traffic -> mixed fidelity simulation
for i in range(1, num_ns3_hosts + 1):
    host = e2e.E2ESimpleNs3Host(f"ns3server-{i}")
    host.delay = "1us"
    host.data_rate = f"{link_rate}Mbps"
    host.ip = f"192.168.64.{i}/24"
    host.queue_size = f"2000000B"
    app = e2e.E2EPacketSinkApplication("sink")
    app.local_ip = "0.0.0.0:5000"
    host.add_component(app)
    topology.add_left_component(host)

for i in range(1, num_ns3_hosts + 1):
    host = e2e.E2ESimpleNs3Host(f"ns3client-{i}")
    host.delay = "1us"
    host.data_rate = f"{link_rate}Mbps"
    host.ip = f"192.168.64.{i+num_ns3_hosts+total_simbricks_hosts}/24"
    host.queue_size = f"2000000B"
    app = e2e.E2EBulkSendApplication("sender")
    app.remote_ip = f"192.168.64.{i}:5000"
    host.add_component(app)
    topology.add_right_component(host)

# create the actual experiment instance
e = exp.Experiment("-".join([h[1] for h in hos_conf]) + "-" + nic_type + "-Host")
e.add_network(net)

# create nic class and node config to use
NicClass, NcClass = get_nic_class_and_node(nic_type)

# create "proper" simulated servers and clients
ip_start = num_ns3_hosts + 1
servers = []
server_index = 1
for hos in hos_conf:
    for i in range(0, hos[0]):
        nic = NicClass()
        nic.name = nic_type
        nic.set_network(net)

        node_config = NcClass()
        node_config.prefix = 24
        ip = ip_start + i
        node_config.ip = f"10.0.{int(ip / 256)}.{ip % 256}"
        node_config.app = node.NetperfServer()

        host_class = get_host_class(e, hos[1])
        server = host_class(node_config)
        server.name = f"s.{hos[1]}.{i}"

        server.add_nic(nic)
        e.add_nic(nic)
        e.add_host(server)

        servers.append(server)

        sn = f"server-{server_index}"
        add_host_to_topo_left(topology, sn, nic, unsynchronized)
        server_index += 1

    ip_start += hos[0]

clients = []
client_index = 1
for hos in hos_conf:
    for i in range(0, hos[0]):
        nic = NicClass()
        nic.name = nic_type
        nic.set_network(net)

        node_config = NcClass()
        node_config.prefix = 24
        ip = ip_start + i
        node_config.ip = f"10.0.{int(ip / 256)}.{ip % 256}"
        node_config.app = node.NetperfClient()

        host_class = get_host_class(e, hos[1])
        client = host_class(node_config)
        client.name = f"c.{hos[1]}.{i}"

        client.add_nic(nic)
        e.add_nic(nic)
        e.add_host(client)

        clients.append(client)

        cn = f"client-{client_index}"
        add_host_to_topo_right(topology, cn, nic, unsynchronized)
        client_index += 1

    ip_start += hos[0]

# tell the clients netperf applications about the server ips they should send to
i = 0
for cl in clients:
    cl.node_config.app.server_ip = servers[i].node_config.ip
    i += 1

# The last client waits for the output printed in other hosts,
# then cleanup
clients[total_simbricks_hosts - 1].node_config.app.is_last = True
clients[total_simbricks_hosts - 1].wait = True

net.init_network()

print(e.name)
experiments.append(e)


# visualize simulation topology
if __name__ == "__main__":
    for e in experiments:
        digraph = experiment_graph_e2e_dumbbell(e.networks[0].e2e_topologies[0])
        digraph.render(filename=f"{e.name}-digraph")
