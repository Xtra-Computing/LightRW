TARGET = hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

test_size:=1
CXXFLAGS += -DTEST_SIZE=${test_size}


include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/graph/Makefile
include host/accelerator/Makefile
include host/pwrs_dump/Makefile


include src/wrs/Makefile
include src/sdyburst/Makefile
include src/srw/Makefile
include src/vdirect/Makefile

include test/test_step_record/Makefile
include test/test_common/Makefile
include test/test_load_weight/Makefile

#include test/test_burst_stream_sink/Makefile
