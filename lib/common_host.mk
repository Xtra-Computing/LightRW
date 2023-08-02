include mk/lib/hls.mk
include mk/lib/xrt.mk
include src/stream/Makefile

CPP_FLAGS += -Wno-unknown-pragmas

include test/test_common/Makefile
include host/kernel/Makefile
