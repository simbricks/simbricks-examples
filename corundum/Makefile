# Copyright 2021 Max Planck Institute for Software Systems, and
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


dir_corundum := ./corundum
verilator_dir_corundum := $(dir_corundum)/obj_dir
verilog_interface_name := mqnic_core_axi
verilator_interface_name := V$(verilog_interface_name)
verilator_src_corundum := $(verilator_dir_corundum)/$(verilator_interface_name).cpp
verilator_bin_corundum := $(verilator_dir_corundum)/$(verilator_interface_name)
adapter_main := corundum_simbricks_adapter
corundum_simbricks_adapter_src := $(adapter_main).cpp
# OBJS := $(corundum_simbricks_adapter_src:.cpp=.o)
# $(OBJS): CPPFLAGS := $(CPPFLAGS) -I$(d)include/
corundum_simbricks_adapter_bin := $(adapter_main)

simbricks_base := /simbricks

mqnic_dir := $(dir_corundum)/modules/mqnic
# kernel_dir := $(wildcard $(simbricks_base)/images/kernel/linux-)
kernel_dir := $(simbricks_base)/images/kernel/linux-5.15.93

lib_dir := $(simbricks_base)/lib
simbricks_lib_dir := $(lib_dir)/simbricks
lib_nicif := $(simbricks_lib_dir)/nicif/libnicif.a
lib_netif := $(simbricks_lib_dir)/netif/libnetif.a 
lib_pcie := $(simbricks_lib_dir)/pcie/libpcie.a 
lib_base := $(simbricks_lib_dir)/base/libbase.a

VERILATOR = verilator
VFLAGS = -Wno-WIDTH -Wno-PINMISSING -Wno-LITENDIAN -Wno-IMPLICIT -Wno-SELRANGE \
    -Wno-CASEINCOMPLETE -Wno-UNSIGNED -Wno-UNOPTFLAT # --no-timing
# VFLAGS = +1364-2005ext+v \
#     -Wno-WIDTH -Wno-PINMISSING -Wno-LITENDIAN -Wno-IMPLICIT -Wno-SELRANGE \
#     -Wno-CASEINCOMPLETE -Wno-UNSIGNED 

TOPLEVEL = mqnic_core_axi


# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/$(TOPLEVEL).v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_core.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_dram_if.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_interface.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_interface_tx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_interface_rx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_port.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_port_tx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_port_rx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_egress.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_ingress.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_l2_egress.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_l2_ingress.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_rx_queue_map.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_ptp.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_ptp_clock.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_ptp_perout.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_rb_clk_info.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/cpl_write.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/cpl_op_mux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/desc_fetch.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/desc_op_mux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/queue_manager.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/cpl_queue_manager.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/tx_fifo.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/rx_fifo.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/tx_req_mux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/tx_engine.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/rx_engine.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/tx_checksum.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/rx_hash.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/rx_checksum.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/stats_counter.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/stats_collect.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/stats_pcie_if.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/stats_pcie_tlp.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/stats_dma_if_axi.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/stats_dma_latency.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/mqnic_tx_scheduler_block_rr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/rtl/tx_scheduler_rr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/mac_ctrl_rx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/mac_ctrl_tx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/mac_pause_ctrl_rx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/mac_pause_ctrl_tx.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/ptp_td_phc.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/ptp_td_leaf.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/eth/rtl/ptp_perout.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_crossbar.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_crossbar_addr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_crossbar_rd.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_crossbar_wr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_reg_if.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_reg_if_rd.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_reg_if_wr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_register_rd.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/axil_register_wr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/arbiter.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axi/rtl/priority_encoder.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_adapter.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_arb_mux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_async_fifo.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_async_fifo_adapter.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_demux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_fifo.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_fifo_adapter.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_pipeline_fifo.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/axis/rtl/axis_register.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/irq_rate_limit.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_axi.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_axi_rd.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_axi_wr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_mux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_mux_rd.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_mux_wr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_if_desc_mux.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_ram_demux_rd.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_ram_demux_wr.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_psdpram.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_client_axis_sink.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/dma_client_axis_source.v
# VERILOG_SOURCES += $(dir_corundum)/fpga/common/lib/pcie/rtl/pulse_merge.v

# # module parameters

# # Structural configuration
# export PARAM_IF_COUNT := 1
# export PARAM_PORTS_PER_IF := 1
# export PARAM_SCHED_PER_IF := $(PARAM_PORTS_PER_IF)

# # Clock configuration
# export PARAM_CLK_PERIOD_NS_NUM := 4
# export PARAM_CLK_PERIOD_NS_DENOM := 1

# # PTP configuration
# export PARAM_PTP_CLK_PERIOD_NS_NUM := 32
# export PARAM_PTP_CLK_PERIOD_NS_DENOM := 5
# export PARAM_PTP_CLOCK_PIPELINE := 0
# export PARAM_PTP_CLOCK_CDC_PIPELINE := 0
# export PARAM_PTP_SEPARATE_TX_CLOCK := 0
# export PARAM_PTP_SEPARATE_RX_CLOCK := 0
# export PARAM_PTP_PORT_CDC_PIPELINE := 0
# export PARAM_PTP_PEROUT_ENABLE := 0
# export PARAM_PTP_PEROUT_COUNT := 1

# # Queue manager configuration
# export PARAM_EVENT_QUEUE_OP_TABLE_SIZE := 32
# export PARAM_TX_QUEUE_OP_TABLE_SIZE := 32
# export PARAM_RX_QUEUE_OP_TABLE_SIZE := 32
# export PARAM_CQ_OP_TABLE_SIZE := 32
# export PARAM_EQN_WIDTH := 6
# export PARAM_TX_QUEUE_INDEX_WIDTH := 13
# export PARAM_RX_QUEUE_INDEX_WIDTH := 8
# export PARAM_CQN_WIDTH := $(shell python -c "print(max($(PARAM_TX_QUEUE_INDEX_WIDTH), $(PARAM_RX_QUEUE_INDEX_WIDTH)) + 1)")
# export PARAM_EQ_PIPELINE := 3
# export PARAM_TX_QUEUE_PIPELINE := $(shell python -c "print(3 + max($(PARAM_TX_QUEUE_INDEX_WIDTH)-12, 0))")
# export PARAM_RX_QUEUE_PIPELINE := $(shell python -c "print(3 + max($(PARAM_RX_QUEUE_INDEX_WIDTH)-12, 0))")
# export PARAM_CQ_PIPELINE := $(shell python -c "print(3 + max($(PARAM_CQN_WIDTH)-12, 0))")

# # TX and RX engine configuration
# export PARAM_TX_DESC_TABLE_SIZE := 32
# export PARAM_RX_DESC_TABLE_SIZE := 32
# export PARAM_RX_INDIR_TBL_ADDR_WIDTH := $(shell python -c "print(min($(PARAM_RX_QUEUE_INDEX_WIDTH), 8))")

# # Scheduler configuration
# export PARAM_TX_SCHEDULER_OP_TABLE_SIZE := $(PARAM_TX_DESC_TABLE_SIZE)
# export PARAM_TX_SCHEDULER_PIPELINE := $(PARAM_TX_QUEUE_PIPELINE)
# export PARAM_TDMA_INDEX_WIDTH := 6

# # Interface configuration
# export PARAM_PTP_TS_ENABLE := 1
# export PARAM_TX_CPL_ENABLE := $(PARAM_PTP_TS_ENABLE)
# export PARAM_TX_CPL_FIFO_DEPTH := 32
# export PARAM_TX_TAG_WIDTH := 16
# export PARAM_TX_CHECKSUM_ENABLE := 1
# export PARAM_RX_HASH_ENABLE := 1
# export PARAM_RX_CHECKSUM_ENABLE := 1
# export PARAM_LFC_ENABLE := 1
# export PARAM_PFC_ENABLE := $(PARAM_LFC_ENABLE)
# export PARAM_MAC_CTRL_ENABLE := 1
# export PARAM_TX_FIFO_DEPTH := 32768
# export PARAM_RX_FIFO_DEPTH := 131072
# export PARAM_MAX_TX_SIZE := 9214
# export PARAM_MAX_RX_SIZE := 9214
# export PARAM_TX_RAM_SIZE := 131072
# export PARAM_RX_RAM_SIZE := 131072

# # RAM configuration
# export PARAM_DDR_CH := 1
# export PARAM_DDR_ENABLE := 0
# export PARAM_DDR_GROUP_SIZE := 1
# export PARAM_AXI_DDR_DATA_WIDTH := 256
# export PARAM_AXI_DDR_ADDR_WIDTH := 32
# export PARAM_AXI_DDR_ID_WIDTH := 8
# export PARAM_AXI_DDR_MAX_BURST_LEN := 256
# export PARAM_HBM_CH := 1
# export PARAM_HBM_ENABLE := 0
# export PARAM_HBM_GROUP_SIZE := $(PARAM_HBM_CH)
# export PARAM_AXI_HBM_DATA_WIDTH := 256
# export PARAM_AXI_HBM_ADDR_WIDTH := 32
# export PARAM_AXI_HBM_ID_WIDTH := 6
# export PARAM_AXI_HBM_MAX_BURST_LEN := 16

# # Application block configuration
# export PARAM_APP_ID := $(shell echo $$((0x00000000)) )
# export PARAM_APP_ENABLE := 0
# export PARAM_APP_CTRL_ENABLE := 1
# export PARAM_APP_DMA_ENABLE := 1
# export PARAM_APP_AXIS_DIRECT_ENABLE := 1
# export PARAM_APP_AXIS_SYNC_ENABLE := 1
# export PARAM_APP_AXIS_IF_ENABLE := 1
# export PARAM_APP_STAT_ENABLE := 1

# # AXI interface configuration (DMA)
# export PARAM_AXI_DATA_WIDTH := 128
# export PARAM_AXI_ADDR_WIDTH := 49
# export PARAM_AXI_STRB_WIDTH := $(shell expr $(PARAM_AXI_DATA_WIDTH) / 8 )
# export PARAM_AXI_ID_WIDTH := 6

# # DMA interface configuration
# export PARAM_DMA_IMM_ENABLE := 0
# export PARAM_DMA_IMM_WIDTH := 32
# export PARAM_DMA_LEN_WIDTH := 16
# export PARAM_DMA_TAG_WIDTH := 16
# export PARAM_RAM_ADDR_WIDTH := $(shell python -c "print((max($(PARAM_TX_RAM_SIZE), $(PARAM_RX_RAM_SIZE))-1).bit_length())")
# export PARAM_RAM_PIPELINE := 2
# export PARAM_AXI_DMA_MAX_BURST_LEN := 16
# export PARAM_AXI_DMA_READ_USE_ID := 0
# export PARAM_AXI_DMA_WRITE_USE_ID := 1
# export PARAM_AXI_DMA_READ_OP_TABLE_SIZE := $(shell echo "$$(( 1 << $(PARAM_AXI_ID_WIDTH) ))" )
# export PARAM_AXI_DMA_WRITE_OP_TABLE_SIZE := $(shell echo "$$(( 1 << $(PARAM_AXI_ID_WIDTH) ))" )

# # Interrupt configuration
# export PARAM_IRQ_COUNT := 32

# # AXI lite interface configuration (control)
# export PARAM_AXIL_CTRL_DATA_WIDTH := 32
# export PARAM_AXIL_CTRL_ADDR_WIDTH := 24
# export PARAM_AXIL_CSR_PASSTHROUGH_ENABLE := 0

# # AXI lite interface configuration (application control)
# export PARAM_AXIL_APP_CTRL_DATA_WIDTH := $(PARAM_AXIL_CTRL_DATA_WIDTH)
# export PARAM_AXIL_APP_CTRL_ADDR_WIDTH := 24

# # Ethernet interface configuration
# export PARAM_AXIS_DATA_WIDTH := 64
# export PARAM_AXIS_SYNC_DATA_WIDTH := $(PARAM_AXIS_DATA_WIDTH)
# export PARAM_AXIS_RX_USE_READY := 0
# export PARAM_AXIS_TX_PIPELINE := 0
# export PARAM_AXIS_TX_FIFO_PIPELINE := 2
# export PARAM_AXIS_TX_TS_PIPELINE := 0
# export PARAM_AXIS_RX_PIPELINE := 0
# export PARAM_AXIS_RX_FIFO_PIPELINE := 2

# # Statistics counter subsystem
# export PARAM_STAT_ENABLE := 1
# export PARAM_STAT_DMA_ENABLE := 1
# export PARAM_STAT_AXI_ENABLE := 1
# export PARAM_STAT_INC_WIDTH := 24
# export PARAM_STAT_ID_WIDTH := 12

# $(verilator_src_corundum): # $(vsrcs_corundum)
# 	$(VERILATOR) $(VFLAGS) --cc -O1 \
# 		$(foreach v,$(filter PARAM_%,$(.VARIABLES)),-G$(subst PARAM_,,$(v))=$($(v))) \
# 	    -CFLAGS "-I$(abspath $(lib_dir)) -iquote $(abspath $(base_dir)) -O1 -g -Wall -Wno-maybe-uninitialized" \
# 	    --Mdir $(verilator_dir_corundum) \
# 		--top-module $(TOPLEVEL) \
# 		--trace \
# 		--exe $(abspath $(corundum_simbricks_adapter_src)) $(abspath $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base)) \
# 		$(VERILOG_SOURCES)

# -CFLAGS "-I$(abspath $(lib_dir)) -iquote $(abspath $(base_dir)) -O3 -g -Wall -Wno-maybe-uninitialized" \

$(verilator_src_corundum): # $(vsrcs_corundum)
	$(VERILATOR) $(VFLAGS) -GPTP_TS_ENABLE=0 -GSTAT_ENABLE=0 -GSTAT_DMA_ENABLE=0 \
		-GSTAT_AXI_ENABLE=0 -GAPP_ENABLE=0 -GAPP_CTRL_ENABLE=0 -GAPP_DMA_ENABLE=0 \
		-GAPP_AXIS_DIRECT_ENABLE=0 -GAPP_AXIS_SYNC_ENABLE=0 -GAPP_AXIS_IF_ENABLE=0 \
		-GAPP_STAT_ENABLE=0 --cc -O3 \
	    -CFLAGS "-I$(abspath $(lib_dir)) -O3 -g -Wall -Wno-maybe-uninitialized" \
	    --Mdir $(verilator_dir_corundum) \
		--top-module $(verilog_interface_name) \
		--trace \
	    -y $(dir_corundum)/fpga/common/rtl \
		-y $(dir_corundum)/fpga/common/lib/axis/rtl \
		-y $(dir_corundum)/fpga/common/lib/eth/rtl \
		-y $(dir_corundum)/fpga/common/lib/pcie/rtl \
	    -y $(dir_corundum)/fpga/lib/axi/rtl \
	    -y $(dir_corundum)/fpga/lib/eth/lib/axis/rtl/ \
	    -y $(dir_corundum)/fpga/lib/pcie/rtl \
	    $(dir_corundum)/fpga/common/rtl/$(verilog_interface_name).v \
	    $(dir_corundum)/fpga/common/rtl/mqnic_tx_scheduler_block_rr.v \
		--exe $(abspath $(corundum_simbricks_adapter_src)) $(abspath $(lib_nicif) $(lib_netif) $(lib_pcie) $(lib_base))

$(verilator_bin_corundum): $(verilator_src_corundum) $(corundum_simbricks_adapter_src)
	$(MAKE) -C $(verilator_dir_corundum) -f $(verilator_interface_name).mk

$(corundum_simbricks_adapter_bin): $(verilator_bin_corundum)
	cp $< $@

adapter: $(corundum_simbricks_adapter_bin)

driver:
	$(MAKE) -C $(kernel_dir) M=$(abspath $(mqnic_dir)) modules

all: driver adapter
.DEFAULT_GOAL := all

clean: 
	rm -rf $(corundum_simbricks_adapter_bin) $(verilator_dir_corundum) $(OBJS) ready

.PHONY: all driver adapter clean


