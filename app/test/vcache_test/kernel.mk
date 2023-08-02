TARGET ?= hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


CPP_FLAGS += -DUSE_LOG
CPP_FLAGS += -DHW_VCACHE



include lib/common_host.mk

include src/rng_data_type/Makefile
include src/rw_data_type/Makefile

include host/log/Makefile
include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/xgraph/Makefile
include host/vcache_test/Makefile

include test/test_load_32/Makefile
include test/test_store_64/Makefile
include src/vdac_cache/Makefile