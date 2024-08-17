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
bdp = int(link_rate * link_latency / 1000 * 10**6)  # Bandwidth-delay product
ip_start = "192.168.64.1"
hos = "qt"
nic_class = sim.CorundumVerilatorNIC
node_class = corundum_linux_node
num_ns3_hosts = 1
unsynchronized = False

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
# create client host and NIC
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
i = 0
for cl in clients:
    cl.node_config.app.server_ip = servers[i].node_config.ip
    i += 1

# The last client waits for the output printed in other hosts,
# then cleanup
clients[0].node_config.app.is_last = True
clients[0].wait = True

net.init_network()

print(e.name)
experiments.append(e)
