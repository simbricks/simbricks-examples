import simbricks.orchestration.e2e_components as e2e
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.simulator_utils import create_basic_hosts
import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
from helpers import (
    add_host_to_topo_left,
    add_host_to_topo_right,
    corundum_linux_node,
    get_host_class,
)

######################################################
# experiment parameters
# -----------------------------------------------------
link_rate = 200  # in Mbps
link_latency = 5  # in ms
bdp = int(link_rate * link_latency / 1000 * 10**6)  # Bandwidth-delay product
ip_start = "192.168.64.1"
hos = "qt"
nic_class = sim.CorundumVerilatorNIC
node_class = corundum_linux_node
unsynchronized = False

######################################################
# create experiment
# -----------------------------------------------------
experiments = []
e = exp.Experiment("qt-Host")

######################################################
# create network
# -----------------------------------------------------
net = sim.NS3E2ENet()
options = {
    "ns3::TcpSocket::SndBufSize": "524288",
    "ns3::TcpSocket::RcvBufSize": "524288",
}
net.opt = " ".join([f"--{o[0]}={o[1]}" for o in options.items()])
# create a dumbbell topology
topology = E2EDumbbellTopology()
topology.data_rate = f"{link_rate}Mbps"
topology.delay = f"{link_latency}ms"
topology.queue_size = f"2000000B"
topology.mtu = f"{1500}"
net.add_component(topology)
e.add_network(net)

######################################################
# create server hosts and NIC
# -----------------------------------------------------
ip_start = 1
servers = create_basic_hosts(
    e,
    1,
    f"ser-{hos}",
    net,
    nic_class,
    get_host_class(e, hos),
    node_class,
    node.NetperfServer,
    ip_start=ip_start,
)
ip_start += 1

######################################################
# create client hosts and NIC
# -----------------------------------------------------
clients = create_basic_hosts(
    e,
    1,
    f"cli-{hos}",
    net,
    nic_class,
    get_host_class(e, hos),
    node_class,
    node.NetperfClient,
    ip_start=ip_start,
)

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
clients[0].node_config.app.server_ip = servers[0].node_config.ip

# The last client waits for the output printed in other hosts,
# then cleanup
clients[0].node_config.app.is_last = True
clients[0].wait = True

net.init_network()

print(e.name)
experiments.append(e)
