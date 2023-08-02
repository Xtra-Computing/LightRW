TARGET = hw
DEVICE = xilinx_u250_gen3x16_xdma_3_1_202020_1
FREQ = 320

include host/main/Makefile
include host/graph/Makefile
include host/pcg/Makefile
include host/xcl2/Makefile
include host/helper/Makefile

include src/rsgu/Makefile
include src/rssink/Makefile
#include src/top/Makefile
#include src/rng/Makefile
include src/sou/Makefile
include src/usample/Makefile
include src/rw/Makefile
include src/vcache/Makefile
