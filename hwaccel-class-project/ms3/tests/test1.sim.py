# TEST 1: Performance test for the hardware accelerated systems with reduced
# output copies when multiplying larger matrices than supported with the
# hardware accelerator with different configured latencies for the accelerator
# to complete the operations. Expect this to take about 5 minutes.

import sys; sys.path.append('./tests/') # add tests dir to module search path
import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim

from hwaccel_common import *

experiments = []

# Run test with multiple different operation latencies for the accelerator
#           10ms         1ms         1us      1ns
for lat in [10000000000, 1000000000, 1000000, 1000]:
  e = exp.Experiment(f'test1-{lat}')
  e.checkpoint = True

  server_config = HwAccelNode()
  server_config.app = MatMulApp(512, 10)

  server = sim.Gem5Host(server_config)
  server.name = 'host'
  server.cpu_type = 'TimingSimpleCPU'
  server.cpu_freq = '1GHz'

  hwaccel = HWAccelSim(lat, 128, 3 * 128 * 128)
  hwaccel.name = 'accel'
  hwaccel.sync = True
  server.add_pcidev(hwaccel)

  e.add_pcidev(hwaccel)
  e.add_host(server)
  server.wait = True

  experiments.append(e)