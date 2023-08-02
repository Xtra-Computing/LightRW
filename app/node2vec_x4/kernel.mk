TARGET ?= hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

TEMPLATE=node2vec
NUMBER_OF_PIPELINE = 4

include mk/pl_mp_kernel.mk