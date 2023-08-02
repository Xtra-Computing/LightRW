TARGET = hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

VPP_FLAGS += -DMEM_BURST_BUFFER_SIZE=2

DEFAULT_CFG=app/test/rn_test_ddr/kernel.cfg
include lib/rn_test/kernel.mk

