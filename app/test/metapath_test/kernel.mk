TARGET ?= hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

include lib/common_host.mk

include src/rng_data_type/Makefile
include src/rw_data_type/Makefile


include host/log/Makefile
include host/xgraph/Makefile

include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile

include host/metapath_host/Makefile

include src/irsgu/Makefile
include src/rwsou/Makefile
#sample
include src/wrs/Makefile
include src/msample/Makefile

include src/sdyburst/Makefile
#
include src/mrw/Makefile

include src/step_loader/Makefile
include src/ring_manager/Makefile
include src/vcache_metapath_test/Makefile
include src/metapath/Makefile

CPP_FLAGS += -DUSE_LOG
CPP_FLAGS += -DACC_NUM=1

EMU_ARGS+= -qsize 1024
