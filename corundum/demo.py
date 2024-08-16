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

# needed for dot graph
import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
from visualize import experiment_graph_e2e_dumbbell

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
        #e.checkpoint = True
    else:
        raise NameError(host_type)
    return HostClass

# helper method to create NicClass and NodeConfig
def get_nic_class_and_node(nic_type):
    # nic
    NicClass = None
    NcClass = None
    if nic_type == "ib":
        NicClass = sim.I40eNIC
        NcClass = node.I40eLinuxNode
    elif nic_type == "cb":
        NicClass = sim.CorundumBMNIC
        NcClass = node.CorundumLinuxNode
    elif nic_type == "cv":
        NicClass = sim.CorundumVerilatorNIC
        NcClass = node.CorundumLinuxNode
    else:
        raise NameError(nic_type)
    return NicClass, NcClass


# parameters for eperiment
mtu = 1500
total_rate = 1000  # Mbps
link_rate = 200  # in Mbps
link_latency = 5  # in ms
bdp = int(link_rate * link_latency / 1000 * 10**6)  # Bandwidth-delay product
ip_start = "192.168.64.1"
num_ns3_hosts = 0
nics_confs = ["cb"] # one of: 'cv', 'cb', 'ib'
hos_conf = [(1, "gt"), (1, "qt")] # (amount, host_type), host_type one of qt, gt, qemu


# create actual experiment(s)
experiments = []

total_simbricks_hosts = 0
for hos in hos_conf:
    total_simbricks_hosts += hos[0]
simbricks_per_client_rate = int(total_rate / total_simbricks_hosts)
rate = f"{simbricks_per_client_rate}m"
for nic_type in nics_confs:
    queue_size = int(bdp * 2**1)
    options = {
        "ns3::TcpSocket::SndBufSize": "524288",
        "ns3::TcpSocket::RcvBufSize": "524288",
    }
    net = sim.NS3E2ENet()
    net.opt = " ".join([f"--{o[0]}={o[1]}" for o in options.items()])

    topology = E2EDumbbellTopology()
    topology.data_rate = f"{link_rate}Mbps"
    topology.delay = f"{link_latency}ms"
    topology.queue_size = f"{queue_size}B"
    topology.mtu = f"{mtu-52}"
    net.add_component(topology)

    # create ns3 applications to create background traffic -> mixed fidelity simulation        
    for i in range(1, num_ns3_hosts + 1):
        host = e2e.E2ESimpleNs3Host(f"ns3server-{i}")
        host.delay = "1us"
        host.data_rate = f"{link_rate}Mbps"
        host.ip = f"192.168.64.{i}/24"
        host.queue_size = f"{queue_size}B"
        app = e2e.E2EPacketSinkApplication("sink")
        app.local_ip = "0.0.0.0:5000"
        app.stop_time = "20s"
        host.add_component(app)
        probe = e2e.E2EPeriodicSampleProbe("probe", "Rx")
        probe.interval = "100ms"
        probe.file = f"sink-rx-{i}"
        app.add_component(probe)
        topology.add_left_component(host)

    for i in range(1, num_ns3_hosts + 1):
        host = e2e.E2ESimpleNs3Host(f"ns3client-{i}")
        host.delay = "1us"
        host.data_rate = f"{link_rate}Mbps"
        host.ip = f"192.168.64.{i+num_ns3_hosts+total_simbricks_hosts}/24"
        host.queue_size = f"{queue_size}B"
        app = e2e.E2EBulkSendApplication("sender")
        app.remote_ip = f"192.168.64.{i}:5000"
        app.stop_time = "20s"
        host.add_component(app)
        topology.add_right_component(host)

    e = exp.Experiment(
        "-".join([h[1] for h in hos_conf])
        + "-"
        + nic_type
        + "-Host-"
        + f"{total_rate}m"
    )
    e.add_network(net)

    # create nic class and node config to use
    NicClass, NcClass = get_nic_class_and_node(nic_type)

    # create "proper" simulated servers and clients
    ip_start = num_ns3_hosts + 1
    servers = []
    for hos in hos_conf:
        servers.extend(
            create_basic_hosts(
                e,
                hos[0],
                f"ser-{hos[1]}",
                net,
                NicClass,
                get_host_class(e, hos[1]),
                NcClass,
                node.NetperfServer,
                ip_start=ip_start,
            )
        )
        ip_start += hos[0]

    clients = []
    for hos in hos_conf:
        clients.extend(
            create_basic_hosts(
                e,
                hos[0],
                f"cli-{hos[1]}",
                net,
                NicClass,
                get_host_class(e, hos[1]),
                NcClass,
                node.NetperfClient,
                ip_start=ip_start, 
            )
        )
        ip_start += hos[0]

    for i, server in enumerate(servers, 1):
        host = e2e.E2ESimbricksHost(f"SB-ser-{i}")
        host.eth_latency = "1us"
        host.simbricks_component = server.nics[0]
        topology.add_left_component(host)

    for i, client in enumerate(clients, 1):
        host = e2e.E2ESimbricksHost(f"SB-cli-{i}")
        host.eth_latency = "1us"
        host.simbricks_component = client.nics[0]
        topology.add_right_component(host)

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