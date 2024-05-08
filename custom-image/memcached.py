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
Example of an experiment with a custom image (built separately throuhg packer,
see the Makefile). Here we set up a client and a server host connected
through a switch. The server runs memcached, and the client the memaslap client.
"""

import os
import typing as tp
from simbricks.orchestration.experiments import Experiment
from simbricks.orchestration.nodeconfig import (
    I40eLinuxNode, NodeConfig, AppConfig
)
from simbricks.orchestration.simulators import (
    Gem5Host, I40eNIC, QemuHost, SwitchNet
)

# define a new application config for our memcached server
class MemcachedServer(AppConfig):

    def run_cmds(self, node: NodeConfig) -> tp.List[str]:
        return ['memcached -u root -t 1 -c 4096']

# define a new application config for our memcached client
class MemcachedClient(AppConfig):

    def __init__(self) -> None:
        super().__init__()
        self.server_ips = ['10.0.0.1']

    def run_cmds(self, node: NodeConfig) -> tp.List[str]:
        servers = [ip + ':11211' for ip in self.server_ips]
        servers = ','.join(servers)
        return [(
            f'memaslap --binary --time 2s --server={servers}'
            f' --thread=1 --concurrency=1'
            f' --tps=1k --verbose'
        )]

mc_image = os.path.abspath('output-memcached/memcached')

experiments = []

for h in ['qemu', 'gem5']:
  e = Experiment(name=f'memcached-{h}')

  if h == 'gem5':
    e.checkpoint = True  # use checkpoint and restore to speed up simulation
    HC = Gem5Host
  else:
    HC = QemuHost

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
  server_config.disk_image = mc_image
      
  server = HC(server_config)
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
  client_config.disk_image = mc_image
  client_config.app.server_ips = [server_config.ip]
  client = HC(client_config)
  client.name = 'client'
  client.wait = True  # wait for client simulator to finish execution
  client.add_nic(client_nic)
  e.add_host(client)

  experiments.append(e)