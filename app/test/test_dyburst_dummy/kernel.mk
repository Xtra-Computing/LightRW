TARGET = hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile


include src/dyburst/Makefile

include test/host_dummy_dyburst/Makefile
include test/test_common/Makefile
include test/dummy_sample/Makefile
include test/dummy_store_32/Makefile
include test/dummy_vloader/Makefile
include test/test_vstrip/Makefile
#include test/test_burst_stream_sink/Makefile
