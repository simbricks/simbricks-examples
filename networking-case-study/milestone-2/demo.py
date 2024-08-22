import simbricks.orchestration.e2e_components as e2e
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.simulator_utils import create_basic_hosts
import sys
import pathlib

workspace_path = pathlib.Path(__file__).parents[2]
sys.path.insert(0, f"{workspace_path}/utils")
import helpers

######################################################
# experiment parameters
# -----------------------------------------------------
link_rate = 200  # in Mbps
link_latency = 5  # in ms
hos = "qemu"
nic_class = sim.CorundumVerilatorNIC
node_class = helpers.corundum_linux_node
synchronized = False

######################################################
# create experiment
# -----------------------------------------------------
experiments = []
e = exp.Experiment("qt-Host")

######################################################
# create network
# -----------------------------------------------------
net = sim.NS3E2ENet()
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
    helpers.get_host_class(e, hos),
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
    helpers.get_host_class(e, hos),
    node_class,
    node.NetperfClient,
    ip_start=ip_start,
)

######################################################
# tell network topology about attached NICs
# -----------------------------------------------------
for server_index, server in enumerate(servers, 1):
    sn = f"server-{server_index}"
    helpers.add_host_to_topo_left(topology, sn, server.nics[0], synchronized)

for client_index, client in enumerate(clients, 1):
    cn = f"client-{client_index}"
    helpers.add_host_to_topo_right(topology, cn, client.nics[0], synchronized)

######################################################
# tell client application about server IPs to use
# -----------------------------------------------------
i = 0
for cl in clients:
    cl.node_config.app.server_ip = servers[i].node_config.ip
    cl.wait = True
    i += 1

net.init_network()

experiments.append(e)
