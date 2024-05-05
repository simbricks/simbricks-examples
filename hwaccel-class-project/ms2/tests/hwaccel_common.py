import os
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node


# System configuration for the simulated machine. Configures commands to run
# as the system boots before the application runs. Specifically this loads the
# vfio-pci kernel driver and registers our PCI vendor and device id with the
# vfio-pci driver.
class HwAccelNode(node.NodeConfig):
    def __init__(self):
        super().__init__()
        self.drivers = []

    def prepare_pre_cp(self):
        l = [
            'mount -t proc proc /proc',
            'mount -t sysfs sysfs /sys',
            'echo 1 > /sys/module/vfio/parameters/enable_unsafe_noiommu_mode',
            'echo 9876 1234 >/sys/bus/pci/drivers/vfio-pci/new_id']
        return super().prepare_pre_cp() + l


# Actual application to run. Includes the command to run, but also makes sure
# the compiled binary gets copied into the disk image of the simulated machine.
class MatMulApp(node.AppConfig):
    def __init__(self, n = None, its = None):
        super().__init__()
        self.n = n
        self.its = its

    def run_cmds(self, node):
        if self.n is None:
          return [f'/tmp/guest/matmul-accel']
        elif self.its is None:
          return [f'/tmp/guest/matmul-accel {self.n}']
        else:
          return [f'/tmp/guest/matmul-accel {self.n} {self.its}']

    def config_files(self):
        # copy binary into host image during prep
        m = {'matmul-accel': open('app/matmul-accel', 'rb')}
        return {**m, **super().config_files()}


# Simulator component for our accelerator model
class HWAccelSim(sim.PCIDevSim):
    sync = True

    def __init__(self, op_latency, matrix_size):
        super().__init__()
        self.op_latency = op_latency
        self.matrix_size = matrix_size

    def run_cmd(self, env):
        cmd = '%s%s %d %d %s %s' % \
            (os.getcwd(), '/accel-sim/sim', self.op_latency, self.matrix_size,
             env.dev_pci_path(self), env.dev_shm_path(self))
        return cmd
