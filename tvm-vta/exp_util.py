import simbricks.orchestration.simulators as sim
import enum


#######################################
# Application configurations
# -------------------------------------


class TvmDeviceType(enum.Enum):
    VTA = "vta"
    CPU = "cpu"


#######################################
# Simulators
# -------------------------------------


class VTADev(sim.PCIDevSim):

    def __init__(self) -> None:
        super().__init__()
        self.clock_freq = 100
        """Clock frequency in MHz"""

    def run_cmd(self, env):
        cmd = (
            "./vta_src/simbricks/vta_simbricks "
            f"{env.dev_pci_path(self)} {env.dev_shm_path(self)} "
            f"{self.start_tick} {self.sync_period} {self.pci_latency} "
            f"{self.clock_freq}"
        )
        return cmd
