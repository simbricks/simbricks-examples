# TEST 3: this is a functional test for the full stack with a larger matrix than
# the accelerator supports. Here the driver should use block-matrix multiply to
# divide this up into smaller operations for the accelerator.

import sys; sys.path.append('./tests/')
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []

e = exp.Experiment(f'test3')

server_config = HwAccelNode()
server_config.app = MatMulApp(512)
server_config.nockp = True

server = sim.Gem5Host(server_config)
server.name = 'host'
server.cpu_type = 'X86KvmCPU'

hwaccel = HWAccelSim(10000, 128)
hwaccel.name = 'accel'
hwaccel.sync = False
server.add_pcidev(hwaccel)

e.add_pcidev(hwaccel)
e.add_host(server)
server.wait = True

experiments.append(e)
