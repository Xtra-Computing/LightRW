
#include "latency_common.h"


unsigned int minRand(unsigned int seed, int load) {
#pragma HLS inline off
    static ap_uint<32> lfsr;

    if (load == 1) lfsr = seed;
    bool b_32 = lfsr.get_bit(32 - 32);
    bool b_22 = lfsr.get_bit(32 - 22);
    bool b_2 = lfsr.get_bit(32 - 2);
    bool b_1 = lfsr.get_bit(32 - 1);
    bool new_bit = b_32 ^ b_22 ^ b_2 ^ b_1;
    lfsr = lfsr >> 1;
    lfsr.set_bit(31, new_bit);

    return lfsr.to_uint();
}
extern "C" {
    void rng_out(stride_addr_stream_t &out_stream)
    {
#pragma HLS interface ap_ctrl_none port=return
        const unsigned int  size =  (1024  * 1024 * 4);
        minRand(16807, 1);
rng_loop: while(1)
        {
            stride_addr_pkg_t pkg;
#pragma HLS PIPELINE II=1
            uint32x1_t rng_data = minRand(31, 0);
            uint32x1_t addr = (rng_data % (size  / 16));
            pkg.data.ap_member(stride_addr_t, addr) = addr;
            out_stream.write(pkg);
        }
    }
}