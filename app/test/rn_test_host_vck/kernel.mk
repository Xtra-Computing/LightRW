TARGET = hw
DEVICE = xilinx_vck5000_gen4x8_qdma_2_202220_1
FREQ = 300

__AIE_SET__ = true

VPP_FLAGS += -DMEM_BURST_BUFFER_SIZE=1

include lib/rn_test/kernel.mk

