TARGET = hw
DEVICE = xilinx_u55c_gen3x16_xdma_3_202210_1
FREQ = 300

VPP_FLAGS += -DMEM_BURST_BUFFER_SIZE=1

include lib/rn_test/kernel.mk

