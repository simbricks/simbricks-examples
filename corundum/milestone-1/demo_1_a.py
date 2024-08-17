import simbricks.orchestration.e2e_components as e2e
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
from simbricks.orchestration.e2e_topologies import E2EDumbbellTopology
from simbricks.orchestration.simulator_utils import create_basic_hosts
import sys

sys.path.insert(0, "/workspaces/simbricks-examples/utils")
from helpers import (
    corundum_linux_node,
    get_host_class,
)

######################################################
# experiment parameters
# -----------------------------------------------------
link_rate = 200  # in Mbps
link_latency = 5  # in ms
ip_start = "192.168.64.1"
hos = "qemu"
nic_class = sim.CorundumBMNIC
node_class = corundum_linux_node
unsynchronized = True

######################################################
# create experiment
# -----------------------------------------------------
experiments = []
e = exp.Experiment("qemu-Host")

######################################################
# create network
# -----------------------------------------------------
net = sim.SwitchNet()
net.sync = False
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
    get_host_class(e, hos),
    node_class,
    node.NetperfClient,
    ip_start=ip_start,
)

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
