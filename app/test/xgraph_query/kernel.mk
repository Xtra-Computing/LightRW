TARGET = hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

include host/log/Makefile
include host/xgraph/Makefile

include host/xgraph_query/Makefile

include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/accelerator/Makefile

CXXFLAGS += -DUSE_LOG


#include test/test_burst_stream_sink/Makefile
