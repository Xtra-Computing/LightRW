TARGET ?= hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


CPP_FLAGS += -DUSE_LOG



include lib/common_host.mk

include src/rng_data_type/Makefile
include src/rw_data_type/Makefile


include host/log/Makefile
include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/xgraph/Makefile
include host/urw/Makefile


include src/irsgu/Makefile
include src/sou/Makefile
include src/ring_manager/Makefile
include src/step_loader/Makefile
include src/vdirect/Makefile
include src/urw/Makefile

include test/step_counter/Makefile

