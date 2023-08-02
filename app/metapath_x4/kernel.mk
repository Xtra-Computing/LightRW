TARGET ?= hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

TEMPLATE=metapath
NUMBER_OF_PIPELINE = 3

include mk/pl_mp_kernel.mk