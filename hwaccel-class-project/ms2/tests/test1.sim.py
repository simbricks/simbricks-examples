# TEST 1: this is a functional test for the full stack. Runs a a matrix multiply
# with the accelerator and compare it to the sw baseline implementation.

import sys; sys.path.append('./tests/')
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []

e = exp.Experiment(f'test1')

server_config = HwAccelNode()
server_config.app = MatMulApp(512)
server_config.nockp = True

server = sim.Gem5Host(server_config)
server.name = 'host'
server.cpu_type = 'X86KvmCPU'

hwaccel = HWAccelSim(10000, 512)
hwaccel.name = 'accel'
hwaccel.sync = False
server.add_pcidev(hwaccel)

e.add_pcidev(hwaccel)
e.add_host(server)
server.wait = True

experiments.append(e)
