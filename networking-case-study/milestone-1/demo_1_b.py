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
hos = "qt"
nic_class = sim.CorundumVerilatorNIC
node_class = helpers.corundum_linux_node
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
    helpers.get_host_class(e, hos),
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
