# TEST 2: Performance test for the hardware accelerated systems comparing
# end-to-end performance of combinatorial and sequential RTL accelerator
# implementations.

import sys; sys.path.append('./tests/') # add tests dir to module search path
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []


for (hw, clk) in [('comb', 9), ('vec', 8)]:
  e = exp.Experiment(f'test2-{hw}-{clk}')
  e.checkpoint = True

  server_config = HwAccelNode()
  server_config.app = MatMulApp(8, 10)

  server = sim.Gem5Host(server_config)
  server.name = 'host'
  server.cpu_type = 'TimingSimpleCPU'
  server.cpu_freq = '1GHz'

  hwaccel = HWAccelSim(hw, clk * 1000)
  hwaccel.name = 'accel'
  hwaccel.sync = True
  server.add_pcidev(hwaccel)

  e.add_pcidev(hwaccel)
  e.add_host(server)
  server.wait = True

  experiments.append(e)