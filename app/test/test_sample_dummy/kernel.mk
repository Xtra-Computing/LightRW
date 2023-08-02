TARGET = hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300



include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile


include src/wrs/Makefile


include test/host_dummy_sample/Makefile
include test/test_common/Makefile
#include test/dummy_sample/Makefile
include test/test_load_bs/Makefile
include test/test_load_weight/Makefile
include test/test_store_32/Makefile
include test/test_vstrip/Makefile

#include test/test_burst_stream_sink/Makefile
