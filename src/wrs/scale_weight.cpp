#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#define NUM_OF_WEIGHTS (16)
#define FLUSH_WINDOW   (8)



// R ~(0,1) < W/ perfix W
// R * perfix W <  W *( 2^32)

extern "C" {
    void scale_weight(      weight_stream_t                 &rng,
                            weight_stream_t                 &perfix_sum,
                            weight_stream_t                 &base_sum,
                            weight_stream_t                 &res
                     ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            volatile uint8_t flush_counter = 0;
            volatile uint8_t notidle = 0;

read: while (1)
            {
                weight_pkg_t pws_pkg;
                if (perfix_sum.read_nb(pws_pkg)) {
                    flush_counter = 0;
                    notidle = 1;
                    weight_pkg_t  rng_pkg = rng.read();
                    weight_pkg_t  base_pkg = base_sum.read();

                    weight_item_t       rng_data = rng_pkg.data;
                    weight_item_t       pws_data = pws_pkg.data;
                    weight_item_t       base_data = base_pkg.data;

                    ap_uint<32>            rngs[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=rngs   dim=0
                    ap_uint<32>            pwss[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=pwss  dim=0
                    ap_uint<32>            basev[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=basev  dim=0

                    ap_uint<64>  scale_res[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=scale_res  dim=0
                    ap_uint<32>  trunc_res[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=trunc_res  dim=0


rngs_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++) {
#pragma HLS UNROLL
                        rngs[i] = rng_data.range(32 * (i + 1) - 1, 32 * i );
                        pwss[i] = pws_data.range(32 * (i + 1) - 1, 32 * i );
                        basev[i] = base_data.range(32 * (i + 1) - 1, 32 * i );
                        scale_res[i] =  rngs[i] * (pwss[i] + basev[i]);
                        trunc_res[i] = scale_res[i].range(63,32);
                    }

                    weight_item_t       w_data_out;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                    {
#pragma HLS UNROLL
                        w_data_out.range(32 * (i + 1) - 1, 32 * i ) = trunc_res[i];
                    }
                    weight_pkg_t       weight_out;
                    weight_out.data = w_data_out;

                    res.write(weight_out);

                }
                else
                {
                    if (notidle == 1)
                    {
                        flush_counter ++;
                    }
                    if (flush_counter > FLUSH_WINDOW)
                    {
                        notidle = 0;
                        break;
                    }
                }
            }
        }
    }

}


