# TEST 3: this is a performance test of the block matrix multiply with 3
# different block sizes.

import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.experiment.experiment_environment as env

class MatMulApp(node.AppConfig):
    def __init__(self, n, block, it=None):
       super().__init__()
       self.n = n
       self.block = block
       self.it = it
    def run_cmds(self, node):
        # application command to run once the host is booted up
        if self.it is None:
          return [f'/tmp/guest/matrixmultiply_block {self.n} {self.block}']
        else:
          return [f'/tmp/guest/matrixmultiply_block {self.n} {self.block} '
                  f'{self.it}']

    def config_files(self, environment: env.ExpEnv):
        # copy matrixmultiply binary into host image during prep
        m = {'matrixmultiply_block': open('app/matrixmultiply_block', 'rb')}
        return {**m, **super().config_files(environment)}

experiments = []

for b in [16, 64, 128]:
  e = exp.Experiment(f'test3-{b}')
  e.checkpoint = True

  server_config = node.NodeConfig()
  server_config.app = MatMulApp(256, b, 10)

  server = sim.Gem5Host(server_config)
  server.name = 'host'
  server.cpu_type = 'TimingSimpleCPU'
  server.cpu_freq = '1GHz'

  e.add_host(server)
  server.wait = True

  experiments.append(e)