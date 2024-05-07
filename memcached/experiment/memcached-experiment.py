# Copyright 2022 Max Planck Institute for Software Systems, and
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
"""
Simple example experiment, which sets up a client and a server host connected
through a switch.

The client pings the server.
"""

from simbricks.orchestration.experiments import Experiment
from simbricks.orchestration.nodeconfig import (
    I40eLinuxNode, MemcachedServer, MemcachedClient
)
from simbricks.orchestration.simulators import Gem5Host, I40eNIC, SwitchNet

e = Experiment(name='memcached-example')
e.checkpoint = True  # use checkpoint and restore to speed up simulation

#### NETWORK ####

# create network
network = SwitchNet()
e.add_network(network)

#### SERVER ####

# server's NIC
server_nic = I40eNIC()
server_nic.set_network(network)
e.add_nic(server_nic)

# create server
server_config = I40eLinuxNode()  # boot Linux with i40e NIC driver
server_config.ip = '10.0.0.2'
server_config.app = MemcachedServer()
server_config.disk_image='/workspaces/simbricks-examples/memcached/output-memcached/memcached'
server = Gem5Host(server_config)
server.name = 'server'
server.add_nic(server_nic)
e.add_host(server)

#### CLIENT ####

# client's NIC
client_nic = I40eNIC()
client_nic.set_network(network)
e.add_nic(client_nic)

# create client
client_config = I40eLinuxNode()  # boot Linux with i40e NIC driver
client_config.ip = '10.0.0.1'
client_config.app = MemcachedClient()
client_config.disk_image='/workspaces/simbricks-examples/memcached/output-memcached/memcached'
client_config.app.server_ips = [server_config.ip]
client = Gem5Host(client_config)
client.name = 'client'
client.wait = True  # wait for client simulator to finish execution
client.add_nic(client_nic)
e.add_host(client)



experiments = [e]
