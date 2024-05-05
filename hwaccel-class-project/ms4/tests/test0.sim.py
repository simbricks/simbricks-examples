# TEST 0: this is a functional test for the full stack with a block matrix
# multiply and asynchrononous queues

import sys; sys.path.append('./tests/')
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []

e = exp.Experiment(f'test0')

server_config = HwAccelNode()
server_config.app = MatMulApp(512, dma=True)
server_config.nockp = True

server = sim.Gem5Host(server_config)
server.name = 'host'
server.cpu_type = 'X86KvmCPU'

hwaccel = HWAccelSim(10000, 128, 6 * 128 * 128)
hwaccel.name = 'accel'
hwaccel.sync = False
server.add_pcidev(hwaccel)

e.add_pcidev(hwaccel)
e.add_host(server)
server.wait = True

experiments.append(e)
