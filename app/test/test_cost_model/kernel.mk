TARGET = hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300



include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/graph/Makefile

include test/host_dynamic_cost/Makefile

#include test/test_burst_stream_sink/Makefile
