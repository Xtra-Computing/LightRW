TARGET ?= hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

include lib/common_host.mk

include host/node2vec_test/Makefile


include src/rng_data_type/Makefile
include src/rw_data_type/Makefile

include src/irsgu/Makefile
include src/rwsou/Makefile
include src/wrs/Makefile
include src/msample/Makefile
include src/staburst/Makefile
include src/siburst/Makefile
include src/setintersetion/Makefile
include src/isne_x16/Makefile
include src/srw/Makefile
include src/vndirect/Makefile


include host/log/Makefile
include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/xgraph/Makefile


include host/probe/Makefile



include test/test_dummy_step/Makefile

CPP_FLAGS += -DUSE_LOG
CPP_FLAGS += -DACC_NUM=1
CPP_FLAGS += -DLATENCY_TEST=0
CPP_FLAGS += -DSTEP_VERIFICATION=1


ifeq "${TARGET}" "hw_emu"

EMU_ARGS+= -qsize 512


endif

