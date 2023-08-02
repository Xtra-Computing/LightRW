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
include src/isne/Makefile
include src/srw/Makefile
include src/vcache/Makefile

CXXFLAGS += -DUSE_LOG
CXXFLAGS += -DACC_NUM=1
CXXFLAGS += -DLATENCY_TEST=0


