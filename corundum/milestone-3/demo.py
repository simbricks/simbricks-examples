import simbricks.orchestration.e2e_components as e2e
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.simulator_utils import create_basic_hosts
from functools import reduce
import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
from helpers import (
    add_host_to_topo_left,
    add_host_to_topo_right,
    corundum_linux_node,
    get_host_class,
    init_ns3_net,
    init_ns3_dumbbell_topo,
)

######################################################
# experiment parameters
# -----------------------------------------------------
link_rate = 200  # in Mbps
link_latency = 5  # in ms
bdp = int(link_rate * link_latency / 1000 * 10**6)  # Bandwidth-delay product
ip_start = "192.168.64.1"
# (amount, host_type), host_type one of qt, gt, qemu,
hos_conf = [(1, "qt"), (1, "gt")]
nic_class = sim.CorundumVerilatorNIC
node_class = corundum_linux_node
total_simbricks_hosts = reduce(lambda prev, cur: prev + cur[0], hos_conf, 0)
unsynchronized = False

######################################################
# create experiment
# -----------------------------------------------------
experiments = []
e = exp.Experiment("-".join([h[1] for h in hos_conf]) + "-Host")

######################################################
# create network
# -----------------------------------------------------
net = sim.NS3E2ENet()
init_ns3_net(net)
# create a dumbbell topology
topology = E2EDumbbellTopology()
init_ns3_dumbbell_topo(topology, link_rate, link_latency)
net.add_component(topology)
e.add_network(net)

######################################################
# create server hosts and NICs
# -----------------------------------------------------
ip_start = 1
servers = []
for hos in hos_conf:
    servers.extend(
        create_basic_hosts(
            e,
            hos[0],
            f"ser-{hos[1]}",
            net,
            nic_class,
            get_host_class(e, hos[1]),
            node_class,
            node.NetperfServer,
            ip_start=ip_start,
        )
    )
    ip_start += hos[0]

######################################################
# create client hosts and NICs
# -----------------------------------------------------
clients = []
for hos in hos_conf:
    clients.extend(
        create_basic_hosts(
            e,
            hos[0],
            f"cli-{hos[1]}",
            net,
            nic_class,
            get_host_class(e, hos[1]),
            node_class,
            node.NetperfClient,
            ip_start=ip_start,
        )
    )
    ip_start += hos[0]

######################################################
# tell network topology about attached NICs
# -----------------------------------------------------
for server_index, server in enumerate(servers, 1):
    sn = f"server-{server_index}"
    add_host_to_topo_left(topology, sn, server.nics[0], unsynchronized)

for client_index, client in enumerate(clients, 1):
    cn = f"client-{client_index}"
    add_host_to_topo_right(topology, cn, client.nics[0], unsynchronized)

######################################################
# tell client application about server IPs to use
# -----------------------------------------------------
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
