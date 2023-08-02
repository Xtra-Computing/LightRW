TARGET ?= hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300




include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile
include host/graph/Makefile
include host/accelerator/Makefile
include host/pwrs_main/Makefile

include src/irsgu/Makefile
include src/rwsou/Makefile
include src/wrs/Makefile
include src/sdyburst/Makefile
include src/srw/Makefile
include src/vdirect/Makefile
include src/secorder/Makefile

CXXFLAGS += -DACC_NUM=1
