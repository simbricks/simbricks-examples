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

from simbricks.orchestration import system
from simbricks.components.i40e import system as i40e_sys
import re

"""
Helper functions that are used within the other files in this folder.
"""

# Custom defined helper function to create an 'I40ELinuxHost' attached to an 'IntelI40eNIC'.
def sys_host_nic(sys, image, ip, hn=None, nn=None):
    host = i40e_sys.I40ELinuxHost(sys)
    host.add_disk(image)
    host.add_disk(system.LinuxConfigDiskImage(sys, host))
    if hn:
        host.name = hn

    nic = i40e_sys.IntelI40eNIC(sys)
    nic.add_ipv4(ip)
    host.connect_pcie_dev(nic)
    if nn:
        nic.name = nn

    return host, nic


def parse_Iperf_line_bytes(line: str) -> float | None:
    pattern = r"(\d+\.?\d*)\s*MBytes"
    match = re.search(pattern, line)
    if not match:
        return None
    m_bytes = float(match.group(1))
    return m_bytes
