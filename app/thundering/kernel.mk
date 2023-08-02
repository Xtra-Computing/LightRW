TARGET ?= hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


CPP_FLAGS += -DUSE_LOG

include lib/common_host.mk

include host/log/Makefile
include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include test/thundering/Makefile


include src/rng_data_type/Makefile


include src/irsgu/Makefile
include src/sou/Makefile
include test/test_store_32/Makefile