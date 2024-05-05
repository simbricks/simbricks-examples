# TEST 0: this is a functional test for the driver. It just reads and prints the
# supported matrix size from the accelerator. Runs two separate configurations
# with different accelerator sizes.
import sys; sys.path.append('./tests/')
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []

for n in [128, 512]:
    e = exp.Experiment(f'test0-{n}')

    server_config = HwAccelNode()
    server_config.app = MatMulApp()
    server_config.nockp = True

    server = sim.Gem5Host(server_config)
    server.name = 'host'
    server.cpu_type = 'X86KvmCPU'

    hwaccel = HWAccelSim(10000, n)
    hwaccel.name = 'accel'
    hwaccel.sync = False
    server.add_pcidev(hwaccel)

    e.add_pcidev(hwaccel)
    e.add_host(server)
    server.wait = True

    experiments.append(e)
