TARGET ?= hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300


include host/log/Makefile
include host/xgraph/Makefile

include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
#include host/graph/Makefile
include host/accelerator/Makefile
include host/xmain/Makefile


include src/irsgu/Makefile
include src/rwsou/Makefile
include src/wrs/Makefile
include src/staburst/Makefile
include src/siburst/Makefile
include src/setintersetion/Makefile
include src/isne_x16/Makefile
include src/srw/Makefile
include src/vdirect/Makefile
include src/step_loader/Makefile
include src/rm/Makefile

include test/test_common/Makefile
include test/test_step_record/Makefile

#include test/test_dummy_step/Makefile

CXXFLAGS += -DUSE_LOG
CXXFLAGS += -DLATENCY_TEST=0
CXXFLAGS += -DSTEP_VERIFICATION=0


