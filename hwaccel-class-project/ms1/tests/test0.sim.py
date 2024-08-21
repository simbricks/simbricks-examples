# TEST 0: this is just a purely functional test of the baseline.
# This should work initially alread and should not take longer than 10-30
# seconds. Reported performance measurements here are not meaningful.

import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.experiment.experiment_environment as env

class MatMulApp(node.AppConfig):
    def run_cmds(self, node):
        # application command to run once the host is booted up
        return ['/tmp/guest/matrixmultiply']

    def config_files(self, environment: env.ExpEnv):
        # copy matrixmultiply binary into host image during prep
        m = {'matrixmultiply': open('app/matrixmultiply', 'rb')}
        return {**m, **super().config_files(environment)}

e = exp.Experiment(f'test0')

server_config = node.NodeConfig()
server_config.app = MatMulApp()
server_config.nockp = True

server = sim.Gem5Host(server_config)
server.name = 'host'
server.cpu_type = 'X86KvmCPU'

e.add_host(server)
server.wait = True

experiments = [e]