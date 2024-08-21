# Copyright 2023 Max Planck Institute for Software Systems, and
# National University of Singapore
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
import os
import glob
import typing as tp

from PIL import Image

import simbricks.orchestration.experiments as exp
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.simulators as sim
import simbricks.orchestration.nodeconfig as node
import simbricks.orchestration.experiment.experiment_environment as expenv


class JpgDSim(sim.PCIDevSim):

    def __init__(self) -> None:
        super().__init__()
        self.name = "jpeg_decoder"

    def run_cmd(self, env: expenv.ExpEnv) -> str:
        return (
            f"{os.path.abspath('jpeg_decoder_verilator')} "
            f"{env.dev_pci_path(self)} {env.dev_shm_path(self)} "
            f"0 {self.sync_period} {self.pci_latency} "
            "jpeg_decoder_waveform "
        )


class JpgDWorkload(node.AppConfig):

    def __init__(
        self,
        pci_dev: str,
        images: tp.List[str],
        dma_src_addr: int,
        dma_dst_addr: int,
        dump_imgs: bool,
        produce_waveform: bool,
    ) -> None:
        super().__init__()
        self.pci_dev = pci_dev
        self.images = images
        self.dma_src_addr = dma_src_addr
        self.dma_dst_addr = dma_dst_addr
        self.dump_imgs = dump_imgs
        self.produce_waveform = produce_waveform

    def prepare_pre_cp(self) -> tp.List[str]:
        return [
            "mount -t proc proc /proc",
            "mount -t sysfs sysfs /sys",
            # enable vfio access to JPEG decoder
            "echo 1 >/sys/module/vfio/parameters/enable_unsafe_noiommu_mode",
            'echo "dead beef" >/sys/bus/pci/drivers/vfio-pci/new_id',
        ]

    def run_cmds(self, node: node.NodeConfig) -> tp.List[str]:
        cmds = []
        for img in self.images:
            with Image.open(img) as loaded_img:
                width, height = loaded_img.size

            # copy image into memory and invoke pci driver
            cmds.extend(
                [
                    f"echo starting decode of image {os.path.basename(img)}",
                    (
                        f"dd if=/tmp/guest/{os.path.basename(img)} bs=4096"
                        " of=/dev/mem"
                        f" seek={self.dma_src_addr} oflag=seek_bytes "
                    ),
                    (
                        "/tmp/guest/pci_driver"
                        f" {self.pci_dev} "
                        f"{self.dma_src_addr} {os.path.getsize(img)} "
                        f"{self.dma_dst_addr} {int(self.produce_waveform)}"
                    ),
                    f"echo finished decode of image {os.path.basename(img)}",
                ]
            )

            # dump the image as base64 to stdout
            if self.dump_imgs:
                cmds.extend(
                    [
                        f"echo image dump begin {width} {height}",
                        (
                            "dd if=/dev/mem iflag=skip_bytes,count_bytes"
                            " bs=4096"
                            # 2 Bytes per pixel
                            f" skip={self.dma_dst_addr} count={width * height * 2} status=none"
                            " | base64"
                        ),
                        "echo image dump end",
                    ]
                )
        return cmds

    def config_files(self, env: expenv.ExpEnv) -> tp.Dict[str, tp.IO]:
        files = {"pci_driver": open("pci_driver", "rb")}
        for img in self.images:
            files[os.path.basename(img)] = open(img, "rb")
        return files


experiments: tp.List[exp.Experiment] = []
for host_var in ["gem5", "qemu"]:
    e = exp.Experiment(f"jpeg_decoder-{host_var}")
    node_cfg = node.NodeConfig()
    # - memmap: reserve 512 MB of main memory for DMA starting at 1 GB
    # - notsc: disable use of TSC register and therefore remove warnings of it
    # being unstable, which messes with the hexdump
    node_cfg.kcmd_append = "memmap=512M!1G notsc"
    dma_src = 1 * 1000**3
    dma_dst = dma_src + 10 * 1000**2
    node_cfg.memory = 2 * 1024
    # images = glob.glob("./test_imgs/444_unopt/*.jpg")
    images = ["test_imgs/444_unopt/medium.jpg"]
    images.sort()
    node_cfg.app = JpgDWorkload(
        "0000:00:00.0",
        images,
        dma_src,
        dma_dst,
        dump_imgs=True,
        produce_waveform=True,
    )

    if host_var == "gem5":
        e.checkpoint = True
        host = sim.Gem5Host(node_cfg)
    elif host_var == "qemu":
        node_cfg.app.pci_dev = "0000:00:02.0"
        host = sim.QemuHost(node_cfg)
    else:
        raise NameError(f"Variant {host_var} is unhandled")
    host.wait = True
    e.add_host(host)

    accel = JpgDSim()
    host.add_pcidev(accel)
    e.add_pcidev(accel)

    # set realistic pci latency
    host.pci_latency = host.sync_period = accel.pci_latency = (
        accel.sync_period
    ) = 400

    experiments.append(e)
