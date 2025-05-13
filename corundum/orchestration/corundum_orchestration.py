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

from __future__ import annotations

import typing as tp
import typing_extensions as tpe
from simbricks.utils import base as utils_base
from simbricks.orchestration import system as sys
from simbricks.orchestration.simulation import base as sim_base
from simbricks.orchestration.simulation import pcidev as sim_pcidev
from simbricks.orchestration.instantiation import base as inst_base


# System Configuration Integration


class CorundumNIC(sys.SimplePCIeNIC):
    def __init__(self, s: sys.System) -> None:
        super().__init__(s)


class CorundumLinuxHost(sys.LinuxHost):
    def __init__(self, sys) -> None:
        super().__init__(sys)
        self.drivers.append("/tmp/guest/mqnic.ko")

    def config_files(self, inst: inst_base.Instantiation) -> dict[str, tp.IO]:
        m = {
            "mqnic.ko": open(
                "/corundum_src/corundum/modules/mqnic/mqnic.ko",
                "rb",
            ),
            "mqnic-dump": open(
                "/corundum_src/corundum/utils/mqnic-dump",
                "rb",
            ),
        }
        return {**m, **super().config_files(inst=inst)}


# Simulation Configuration Integration


class CorundumVerilatorNICSim(sim_pcidev.NICSim):

    def __init__(self, simulation: sim_base.Simulation):
        super().__init__(
            simulation=simulation,
            executable="/corundum_src/adapter/corundum_simbricks_adapter",
        )
        self.name = f"CorundumVerilatorNICSim-{self._id}"
        self.clock_freq = 250  # MHz

    def resreq_mem(self) -> int:
        # this is a guess
        return 512

    def run_cmd(self, inst: inst_base.Instantiation) -> str:
        nic_devices = self.filter_components_by_type(ty=sys.SimplePCIeNIC)
        nic_device = nic_devices[0]

        channels = self.get_channels()

        pci_channels = sim_base.Simulator.filter_channels_by_sys_type(
            channels, sys.PCIeChannel
        )
        pci_latency, pci_sync_period, pci_run_sync = (
            sim_base.Simulator.get_unique_latency_period_sync(pci_channels)
        )
        socket = inst.get_socket(interface=nic_device._pci_if)
        pci_params_url = self.get_parameters_url(
            inst,
            socket,
            sync=pci_run_sync,
            latency=pci_latency,
            sync_period=pci_sync_period,
        )

        eth_channels = sim_base.Simulator.filter_channels_by_sys_type(
            channels, sys.EthChannel
        )
        eth_latency, eth_sync_period, eth_run_sync = (
            sim_base.Simulator.get_unique_latency_period_sync(eth_channels)
        )
        socket = inst.get_socket(interface=nic_device._eth_if)
        eth_params_url = self.get_parameters_url(
            inst,
            socket,
            sync=eth_run_sync,
            latency=eth_latency,
            sync_period=eth_sync_period,
        )

        cmd = f"{self._executable} {pci_params_url} {eth_params_url} {self._start_tick} {self.clock_freq}"
        return cmd

    def toJSON(self) -> dict:
        json_obj = super().toJSON()
        json_obj["clock_freq"] = self.clock_freq
        return json_obj

    @classmethod
    def fromJSON(
        cls, simulation: sim_base.Simulation, json_obj: dict
    ) -> tpe.Self:
        instance = super().fromJSON(simulation, json_obj)
        instance.clock_freq = utils_base.get_json_attr_top(json_obj, "clock_freq")
        return instance
