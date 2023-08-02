TARGET = hw_emu
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 300

include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile

include src/isne_x16/Makefile

include test/host_adj/Makefile
include test/test_common/Makefile
include test/test_store_32/Makefile
include test/test_load_512/Makefile
include test/test_load_32/Makefile
