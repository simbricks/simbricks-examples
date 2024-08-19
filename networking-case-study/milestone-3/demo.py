import simbricks.orchestration.e2e_components as e2e
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.simulator_utils import create_basic_hosts
import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
import helpers

######################################################
# experiment parameters
# -----------------------------------------------------
link_rate = 200  # in Mbps
link_latency = 5  # in ms
bdp = int(link_rate * link_latency / 1000 * 10**6)  # Bandwidth-delay product
# (amount, host_type), host_type one of qt, gt, qemu,
hos_conf = [(1, "qt"), (1, "gt")]
nic_class = sim.CorundumVerilatorNIC
node_class = helpers.corundum_linux_node
synchronized = True

######################################################
# create experiment
# -----------------------------------------------------
experiments = []
e = exp.Experiment("-".join([h[1] for h in hos_conf]) + "-Host")

######################################################
# create network
# -----------------------------------------------------
net = sim.NS3E2ENet()
# create a dumbbell topology
topology = E2EDumbbellTopology()
helpers.init_ns3_dumbbell_topo(topology, link_rate, link_latency)
net.add_component(topology)
e.add_network(net)

######################################################
# create server hosts and NICs
# -----------------------------------------------------
ip_start = 1
servers = []
for amount, sim_type in hos_conf:
    servers.extend(
        create_basic_hosts(
            e,
            amount,
            f"ser-{sim_type}",
            net,
            nic_class,
            helpers.get_host_class(e, sim_type),
            node_class,
            node.NetperfServer,
            ip_start=ip_start,
        )
    )
    ip_start += amount

######################################################
# create client hosts and NICs
# -----------------------------------------------------
clients = []
for amount, sim_type in hos_conf:
    clients.extend(
        create_basic_hosts(
            e,
            amount,
            f"cli-{sim_type}",
            net,
            nic_class,
            helpers.get_host_class(e, sim_type),
            node_class,
            node.NetperfClient,
            ip_start=ip_start,
        )
    )
    ip_start += amount

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
