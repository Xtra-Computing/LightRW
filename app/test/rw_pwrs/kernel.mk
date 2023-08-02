TARGET ?= hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/graph/Makefile
include host/accelerator/Makefile
include host/pwrs_main/Makefile

include src/rsgu/Makefile
include src/rwsou/Makefile
include src/wrs/Makefile
include src/dyburst/Makefile
include src/rw/Makefile
include src/vcache/Makefile

include test/test_vstrip/Makefile

#include test/test_burst_stream_sink/Makefile
