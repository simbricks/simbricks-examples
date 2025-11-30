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
                # "/corundum_src/corundum/modules/mqnic/mqnic.ko",
                "/workspaces/simbricks-examples-tmp/corundum_legacy/mqnic_driver/mqnic.ko",
                "rb",
            ),
        }
        return {**m, **super().config_files(inst=inst)}


# Simulation Configuration Integration


class CorundumBMNICSim(sim_pcidev.NICSim):

    def __init__(self, simulation: sim_base.Simulation):
        super().__init__(
            simulation=simulation,
            executable="../workspaces/simbricks-examples-tmp/corundum_legacy/behavioral_model/corundum_bm",
        )
        self.name = f"CorundumBMNICSim-{self._id}"

    def toJSON(self) -> dict:
        json_obj = super().toJSON()
        return json_obj

    @classmethod
    def fromJSON(cls, simulation: sim_base.Simulation, json_obj: dict) -> tpe.Self:
        instance = super().fromJSON(simulation, json_obj)
        return instance

    def add(self, nic: CorundumNIC):
        utils_base.has_expected_type(nic, CorundumNIC)
        super().add(nic)

    def run_cmd(self, inst: inst_base.Instantiation) -> str:
        cmd = super().run_cmd(inst)
        if self.mac:
            cmd += " " + ("".join(reversed(self.mac.split(":"))))
            if self.log_file:
                cmd += f" {self.log_file}"
        return cmd


class CorundumVerilatorNICSim(sim_pcidev.NICSim):

    def __init__(self, simulation: sim_base.Simulation):
        super().__init__(
            simulation=simulation,
            # executable="/corundum_src/adapter/corundum_simbricks_adapter",
            executable="../workspaces/simbricks-examples-tmp/corundum_legacy/rtl_model/corundum_verilator",
        )
        self.name = f"CorundumVerilatorNICSim-{self._id}"
        self.clock_freq = 250  # MHz
        # self.clock_freq = 10  # MHz

    def resreq_mem(self) -> int:
        # this is a guess
        return 512

    def add(self, nic: CorundumNIC):
        utils_base.has_expected_type(nic, CorundumNIC)
        super().add(nic)

    def run_cmd(self, inst: inst_base.Instantiation) -> str:
        cmd = super().run_cmd(inst)
        cmd += str(self.clock_freq)
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
