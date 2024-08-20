# TEST 1: this is a performance test of the baseline.
# This should work initially already, and will take a bit longer than the purely
# functional test running at interactive speed. Expect about 10-15 mins.

import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node
from simbricks.orchestration.experiment.experiment_environment import ExpEnv

class MatMulApp(node.AppConfig):
    def run_cmds(self, node):
        # application command to run once the host is booted up
        return ['m5 resetstats', '/tmp/guest/matrixmultiply', 'm5 dumpstats',]

    def config_files(self, env: ExpEnv):
        # copy matrixmultiply binary into host image during prep
        m = {'matrixmultiply': open('app/matrixmultiply', 'rb')}
        return {**m, **super().config_files(env)}

e = exp.Experiment(f'test1')
e.checkpoint = True

server_config = node.NodeConfig()
server_config.app = MatMulApp()

server = sim.Gem5Host(server_config)
server.name = 'host'
server.cpu_type = 'TimingSimpleCPU'
server.cpu_freq = '1GHz'

e.add_host(server)
server.wait = True

experiments = [e]