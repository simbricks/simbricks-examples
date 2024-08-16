# Copyright 2021 Max Planck Institute for Software Systems, and
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

import simbricks.orchestration.e2e_components as e2e

def add_host_to_topo_left(topo, host_name, nic, unsynchronized):
    host = e2e.E2ESimbricksHost(host_name)
    if unsynchronized:
        host.sync = e2e.SimbricksSyncMode.SYNC_DISABLED
    host.eth_latency = "1us"
    host.simbricks_component = nic
    topo.add_left_component(host)

def add_host_to_topo_right(topo, host_name, nic, unsynchronized):
    host = e2e.E2ESimbricksHost(host_name)
    if unsynchronized:
        host.sync = e2e.SimbricksSyncMode.SYNC_DISABLED
    host.eth_latency = "1us"
    host.simbricks_component = nic
    topo.add_right_component(host)