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
    void reservoir_mask(    weight_stream_t                 &weight,
                            weight_stream_t                 &scale,
                            rs_mask_stream_t                &res
                     ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            volatile uint8_t flush_counter = 0;
            volatile uint8_t notidle = 0;

read: while (1)
            {
                weight_pkg_t  weight_pkg;
                if (weight.read_nb(weight_pkg)) {
                    flush_counter = 0;
                    notidle = 1;

                    weight_pkg_t scale_pkg = scale.read();

                    weight_item_t       weight_data = weight_pkg.data;
                    weight_item_t       scale_data = scale_pkg.data;

                    ap_uint<32>            weights[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=weights dim=0
                    ap_uint<32>            scales[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=scales  dim=0

                    ap_uint<1>  masks[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=masks  dim=0


rngs_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++) {
#pragma HLS UNROLL
                        weights[i] = weight_data.range(32 * (i + 1) - 1, 32 * i );
                        scales[i] = scale_data.range(32 * (i + 1) - 1, 32 * i );
                        if ( scales[i] <  weights[i])
                        {
                            masks[i] = 1;
                        }
                        else
                        {
                            masks[i] = 0;
                        }
                    }

                    rs_mask_item_t       rs_mask_out;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                    {
#pragma HLS UNROLL
                        rs_mask_out.range(i ,i) = masks[i];
                    }
                    rs_mask_pkg_t   rs_mask_pkg;
                    rs_mask_pkg.data = rs_mask_out;

                    res.write(rs_mask_pkg);

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