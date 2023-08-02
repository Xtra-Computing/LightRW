#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

typedef ap_uint<16>  mask_t;

const mask_t mask_lut[16] = {
    0x0000, 0x0001, 0x0003, 0x0007,
    0x000f, 0x001f, 0x003f, 0x007f,
    0x00ff, 0x01ff, 0x03ff, 0x07ff,
    0x0fff, 0x1fff, 0x3fff, 0x7fff
};

const mask_t inv_mask_lut[16] = {
    0x0001, 0x0003, 0x0007, 0x000f,
    0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff,
    0x1fff, 0x3fff, 0x7fff, 0xffff
};


//                    inclusive       inclusive
inline mask_t gen_mask_in_16(uint32_t start, uint32_t end)
{
#pragma HLS BIND_STORAGE variable=inv_mask_lut type=rom_np impl=lutram latency=1
#pragma HLS BIND_STORAGE variable=mask_lut type=rom_np impl=lutram latency=1
#pragma HLS INLINE
    uint8_t s = start & 0xf;
    uint8_t e = end & 0xf;
    mask_t s_mask = ~mask_lut[s];
    mask_t e_mask = inv_mask_lut[e];
    return s_mask & e_mask;
}


extern "C" {
    void mask_generator(    mask_cmd_stream_t           &mask_cmd,
                            weight_mask_stream_t        &mask_output
                       ) {
#pragma HLS interface ap_ctrl_none port=return

// inclusive  exclusive
// start      end
mask_gen_main: while (1)
        {

#pragma HLS LATENCY max=0 min=0
            mask_cmd_pkg_t in_pkg = mask_cmd.read();
            mask_cmd_item_t mcmd = in_pkg.data;

            uint32_t start = mcmd.ap_member(mask_cmd_t, start);
            uint32_t end = mcmd.ap_member(mask_cmd_t, end);

            uint32_t start_addr = start & 0xfffffff0;
            uint32_t curr_e_addr = start_addr;
            uint32_t temp = start;
            uint32_t inclusive_end = end - 1;

            uint32_t temp_end_addr = end & 0xfffffff0;
            uint32_t end_addr;
            if (temp_end_addr == end)
            {
                end_addr = temp_end_addr;
            }
            else
            {
                end_addr =  temp_end_addr + 16;
            }

            uint32_t curr_e_in = start_addr - 1 + 16;

mask_gen: while (1)
            {
#pragma HLS LATENCY max=1 min=1
                curr_e_addr += 16;
                uint32_t s = temp;
                uint32_t e = curr_e_addr - 1 ;

                if (curr_e_in > inclusive_end)
                {
                    curr_e_in = inclusive_end;
                }


                weight_mask_item_t wm = gen_mask_in_16(s, curr_e_in);
                weight_mask_pkg_t out_pkg;
                out_pkg.data = wm;
                out_pkg.last = in_pkg.last;
                mask_output.write(out_pkg);

                temp += (curr_e_addr - s);
                curr_e_in += 16;


                if (curr_e_addr == end_addr)
                {
                    break;
                }
            }
        }

    }
}
