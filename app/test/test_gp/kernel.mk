TARGET = hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

include lib/common_host.mk


include host/pcg/Makefile
include host/xcl2/Makefile
include host/gp/Makefile


include src/gp/Makefile
include src/pr/Makefile

include test/test_padding_512/Makefile


VPP_FLAGS += -DLOG_CACHEUPDATEBURST=3
CPP_FLAGS += -DTEST_GP=0