import os
import sys

sys.path.append(str(os.path.dirname(__file__)))

import simbricks.orchestration.experiments as exp
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.experiment.experiment_environment as env
import exp_util
import itertools
import os

experiments = []

# Experiment parameters
host_variants = ["qemu_kvm", "qemu_icount"]
inference_device_opts = [exp_util.TvmDeviceType.CPU, exp_util.TvmDeviceType.VTA]
vta_clk_freq_opts = [100, 400]

# Build experiment for all combinations of parameters
for host_var, inference_device, vta_clk_freq in itertools.product(
    host_variants, inference_device_opts, vta_clk_freq_opts
):
    experiment = exp.Experiment(
        f"detect_simple-{inference_device.value}-{host_var}-{vta_clk_freq}"
    )
    pci_vta_id = 2
    sync = False
    if host_var == "qemu_kvm":
        HostClass = sim.QemuHost
    elif host_var == "qemu_icount":
        HostClass = sim.QemuIcountHost
        sync = True

    #######################################################
    # Define application
    # -----------------------------------------------------

    class TvmDetectLocal(node.AppConfig):
        """Runs inference for detection model locally, either on VTA or the CPU."""

        def __init__(self):
            super().__init__()
            self.pci_device_id = f"0000:00:{(pci_vta_id):02d}.0"
            self.device = inference_device
            self.test_img = "person.jpg"
            self.repetitions = 1
            self.debug = False
            """Whether to dump inference result."""

        def config_files(self, environment: env.ExpEnv):
            # mount TVM inference script in simulated server under /tmp/guest
            return {
                "deploy_detection-infer.py": open(
                    "./tvm_deploy_detection-infer.py", "rb"
                )
            }

        def run_cmds(self, node):
            # define commands to run on simulated server
            cmds = [
                # start RPC server
                f"VTA_DEVICE={self.pci_device_id} python3 -m"
                " vta.exec.rpc_server &"
                # wait for RPC server to start
                "sleep 6",
                f"export VTA_RPC_HOST=127.0.0.1",
                f"export VTA_RPC_PORT=9091",
                # run inference
                (
                    "python3 /tmp/guest/deploy_detection-infer.py "
                    f"/root/darknet {self.device.value} {self.test_img} "
                    f"{self.repetitions} {int(self.debug)}"
                ),
            ]

            # dump image with detection boxes as base64 to allow later inspection
            if self.debug:
                cmds.extend(
                    [
                        "echo dump deploy_detection-infer-result.png START",
                        "base64 deploy_detection-infer-result.png",
                        "echo dump deploy_detection-infer-result.png END",
                    ]
                )
            return cmds

    #######################################################
    # Define server configuration
    # -----------------------------------------------------

    class VtaNode(node.NodeConfig):

        def __init__(self) -> None:
            super().__init__()
            # Use locally built disk image
            self.disk_image = os.path.abspath("./output-tvm/tvm")
            # Bump amount of system memory
            self.memory = 3 * 1024
            # Reserve physical range of memory for the VTA user-space driver
            self.kcmd_append = " memmap=512M!1G"

        def prepare_pre_cp(self):
            # Define commands to run before application to configure the server
            cmds = super().prepare_pre_cp()
            cmds.extend(
                [
                    "mount -t proc proc /proc",
                    "mount -t sysfs sysfs /sys",
                    # Make TVM's Python framework available
                    "export PYTHONPATH=/root/tvm/python:${PYTHONPATH}",
                    "export PYTHONPATH=/root/tvm/vta/python:${PYTHONPATH}",
                    # Set up loopback interface so the TVM inference script can
                    # connect to the RPC server
                    "ip link set lo up",
                    "ip addr add 127.0.0.1/8 dev lo",
                    # Make VTA device available for control from user-space via
                    # VFIO
                    (
                        "echo 1"
                        " >/sys/module/vfio/parameters/enable_unsafe_noiommu_mode"
                    ),
                    'echo "dead beef" >/sys/bus/pci/drivers/vfio-pci/new_id',
                ]
            )
            return cmds

    #######################################################
    # Define and connect all simulators
    # -----------------------------------------------------
    # Instantiate server
    server_cfg = VtaNode()
    server_cfg.app = TvmDetectLocal()
    server = HostClass(server_cfg)
    # Whether to synchronize VTA and server
    server.sync = sync
    # Wait until server exits
    server.wait = True

    # Instantiate and connect VTA PCIe-based accelerator to server
    vta = exp_util.VTADev()
    vta.clock_freq = vta_clk_freq
    server.add_pcidev(vta)

    # Add both simulators to experiment
    experiment.add_host(server)
    experiment.add_pcidev(vta)

    experiments.append(experiment)
