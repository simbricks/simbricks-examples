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
hos = "qemu"
nic_class = sim.CorundumBMNIC
node_class = helpers.corundum_linux_node
synchronized = False

######################################################
# create experiment
# -----------------------------------------------------
experiments = []
e = exp.Experiment("qemu-Host")

######################################################
# create network
# -----------------------------------------------------
net = sim.SwitchNet()
net.sync = synchronized
e.add_network(net)

######################################################
# create server host and NIC
# -----------------------------------------------------
ip_start = 1
servers = []
nic = sim.CorundumBMNIC()
nic.name = "cor-bm"
nic.set_network(net)

node_config = node.CorundumLinuxNode()
node_config.memory = 2048
node_config.prefix = 24
node_config.ip = f"10.0.0.1"
node_config.app = node.NetperfServer()

server = sim.QemuHost(node_config)
server.name = f"s.qemu.1"

server.add_nic(nic)
e.add_nic(nic)
e.add_host(server)

servers.append(server)
ip_start += 1

######################################################
# create client hosts and NICs
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
# tell client application about server IPs to use
# -----------------------------------------------------
i = 0
for cl in clients:
    cl.node_config.app.server_ip = servers[i].node_config.ip
    cl.wait = True
    i += 1

net.init_network()

experiments.append(e)
