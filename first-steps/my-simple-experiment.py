import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node

e = exp.Experiment('my-simple-experiment')

net = sim.SwitchNet()
net.sync = False
e.add_network(net)

server_config = node.I40eLinuxNode()
server_config.prefix = 24
server_config.ip = '10.0.0.1'

server_config.app = node.IperfTCPServer()

server = sim.QemuHost(server_config)
server.name = 'server'
e.add_host(server)

server_nic = sim.I40eNIC()
server_nic.set_network(net)
server.add_nic(server_nic)
e.add_nic(server_nic)


# Node configuration describes software configuration running on the client
client_config = node.I40eLinuxNode()
client_config.prefix = 24
client_config.ip = '10.0.0.2'

client_config.app = node.IperfTCPClient()
client_config.app.server_ip = '10.0.0.1' # Client needs to know server IP

# Create host simulator for client
client = sim.QemuHost(client_config)
client.name = 'client'
client.wait = True # Wait for client to shut down before ending experiment
e.add_host(client)

# Create nic simulator for client
client_nic = sim.I40eNIC()
client_nic.set_network(net)
client.add_nic(client_nic)
e.add_nic(client_nic)

experiments = [e]