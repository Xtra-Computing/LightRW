#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rng_type.h"

#include <ap_int.h>

unsigned int minRand(unsigned int seed, int load) {
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
    void rn_cmd_gen(uint32_t size,
                    uint32_t in_seed,
                    mem_access_cmd_stream_t &output
                   )
    {
#pragma HLS INTERFACE s_axilite port = size bundle = control
#pragma HLS INTERFACE s_axilite port = in_seed bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

        minRand(in_seed, 1);
        for (int i = 0 ; i < size; i++)
        {
#pragma HLS PIPELINE II=1
            uint32_t seed = minRand(31, 0);
            uint32_t one_read = seed & ((1024u * 512u) - 1u);
            mem_access_cmd_pkg_t  cmd_pkg;
            mem_access_cmd_item_t  cmd;
            cmd.ap_member(mem_access_cmd_t, addr)  = one_read;

            if (i == size - 1)
                cmd.ap_member(mem_access_cmd_t, state) = MM_STATE_MASK_END;
            else
                cmd.ap_member(mem_access_cmd_t, state) = 0;

            cmd_pkg.data = cmd;
            cmd_pkg.last = 0;
            output.write(cmd_pkg);
        }
    }

}

