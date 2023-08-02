TARGET = hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


include host/graph/Makefile

include host/graph_converter_main/Makefile

#include test/test_burst_stream_sink/Makefile
