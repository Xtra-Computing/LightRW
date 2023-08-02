TARGET ?= hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

TEMPLATE=rw_node_si
NUMBER_OF_PIPELINE = 2

include mk/multi_pipeline_rule.mk