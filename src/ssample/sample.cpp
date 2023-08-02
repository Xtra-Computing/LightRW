#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#define NUM_OF_WEIGHTS (16)


#define FLUSH_WINDOW  8


#define MSB_OF_4BIT(var, start)   if (masks[(const uint8_t)start] == 1){        \
                        var = (const uint8_t) start + 1;                        \
                     } else if (masks[(const uint8_t)(start - 1)] == 1){        \
                        var = (const uint8_t) start + 0;                        \
                     } else if (masks[(const uint8_t)(start - 2)] == 1){        \
                        var = (const uint8_t) start - 1;                        \
                     } else if (masks[(const uint8_t)(start - 3)] == 1){        \
                        var = (const uint8_t) start - 2;                        \
                     }else{                                                     \
                        var = 0;}

// R ~(0,1) < W/ perfix W
// R * perfix W <  W *( 2^32)

extern "C" {
    void sample  ( burst_sync_stream_t             &burst_sync,
                   rs_mask_stream_t                &mask,
                   weight_stream_t                 &weight,
                   next_vertex_ordered_stream_t    &next_vertex
                 ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            volatile uint8_t  flush_counter = 0;
            volatile uint32_t  selected_index = 0x80000000;
read: while (1)
            {
                weight_pkg_t    weight_pkg;

                burst_sync_pkg_t  sync_pkg;
                if (burst_sync.read_nb(sync_pkg)) {
                    flush_counter = 0;
                    rs_mask_pkg_t mask_pkg = mask.read();
                    weight_pkg = weight.read();
                    ap_uint<1>  masks[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=masks  dim=0

mask_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                    {
#pragma HLS UNROLL
                        masks[i] = mask_pkg.data.range(i , i);
                    }


                    uint32_t            pps_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=pps_data   dim=0

pps_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                    {
#pragma HLS UNROLL
                        pps_data[i] = weight_pkg.data.range(32 * (i + 1) - 1, 32 * i );
                    }

                    ap_uint<5> curr_msb_0, curr_msb_1, curr_msb_2, curr_msb_3;

                    MSB_OF_4BIT(curr_msb_0, 15)
                    MSB_OF_4BIT(curr_msb_1, 11)
                    MSB_OF_4BIT(curr_msb_2, 7)
                    MSB_OF_4BIT(curr_msb_3, 3)

                    ap_uint<5> msb_level_1_0, msb_level_1_1;
                    if (curr_msb_0 > curr_msb_1)
                        msb_level_1_0 = curr_msb_0;
                    else
                        msb_level_1_0 = curr_msb_1;

                    if (curr_msb_2 > curr_msb_3)
                        msb_level_1_1 = curr_msb_2;
                    else
                        msb_level_1_1 = curr_msb_3;

                    ap_uint<5> msb_bit;

                    if (msb_level_1_0 > msb_level_1_1)
                        msb_bit = msb_level_1_0;
                    else
                        msb_bit = msb_level_1_1;

                    if (msb_bit != 0)
                    {
                        selected_index =  pps_data[msb_bit - 1];
                        //selected_index = sync_pkg.data.ap_member(burst_sync_t, addr) + (msb_bit - 1);
                    }


                    if (sync_pkg.data.ap_member(burst_sync_t, burst_id) & 0x8000)
                    {
                        next_vertex_ordered_pkg_t out_pkg;
                        out_pkg.data.ap_member(next_vertex_ordered_t, vertex) = (selected_index & 0x7fffffff); // TODO
                        out_pkg.data.ap_member(next_vertex_ordered_t, id)
                            = sync_pkg.data.ap_member(burst_sync_t, id);
                        out_pkg.last = sync_pkg.last;
                        next_vertex.write(out_pkg);
                        selected_index = 0x80000000;
                    }
                }
                else
                {
                    flush_counter ++;
                    if (flush_counter > FLUSH_WINDOW)
                    {
                        //break;
                    }
                }
            }
        }
    }
}



#if 0


extern "C" {
    void perfix_sum ( burst_sync_stream_t             &burst_sync,
                      weight_stream_t                 &weight_input,
                      next_vertex_ordered_stream_t    &next_vertex
                    ) {
#pragma HLS interface ap_ctrl_none port=return

        weight_inner_stream_t                       w_inner;
#pragma HLS STREAM variable=w_inner depth=4

        burst_sync_frp_inner_stream_t               bs_sync;
#pragma HLS STREAM variable=bs_sync depth=16

        weight_inner_stream_t                       w_sum_inner;
#pragma HLS STREAM variable=w_sum_inner depth=16

        next_vertex_ordered_frp_inner_stream_t      nv_inner;
#pragma HLS STREAM variable=nv_inner depth=16

        for (;;)
        {
#pragma HLS DATAFLOW
            axis_to_hls_stream_frp_with_last(burst_sync, bs_sync);
            axis_to_hls_stream_frp(weight_input, w_inner);
            partial_perfix_sum(w_inner, w_sum_inner);
            dummy_update(bs_sync, w_sum_inner, nv_inner);
            hls_to_axis_stream_frp_with_last(nv_inner, next_vertex);
        }
    }
}


template <int T>



#pragma HLS interface ap_ctrl_none port=return

#pragma HLS DATAFLOW
weight_inner_stream_t w;
#pragma HLS STREAM variable=w depth=2
burst_sync_inner_stream_t  bs;
#pragma HLS STREAM variable=bs depth=2
axis_to_hls_stream(burst_sync, bs);
axis_to_hls_stream(weight_input, w);

burst_sync_inner_stream_t  bs_perfix, bx_sample;
#pragma HLS STREAM variable=bx_sample depth=16
#pragma HLS STREAM variable=bs_perfix depth=2

stream_duplicator(bs, bs_perfix, bx_sample);

weight_inner_stream_t w_out;
#pragma HLS STREAM variable=bs_perfix depth=2
partial_perfix_sum(w, bs_perfix, w_out);
next_vertex_ordered_inner_stream_t nv;
#pragma HLS STREAM variable=nv depth=16
dummy_update(w_out, bx_sample, nv);
hls_to_axis_stream(nv, update);
}
}

#endif
#if 0

free_run: while (1)
{
    volatile uint8_t flush_counter = 0;
read: while (1)
    {

        burst_sync_pkg_t  sync_pkg;
        if (burst_sync.read_nb(sync_pkg)) {
            flush_counter = 0;
            weight_pkg_t weight_pkg = weight_input.read();

            if (sync_pkg.data.ap_member(burst_sync_t, burst_id) & 0x8000)
            {
                next_vertex_ordered_pkg_t out_pkg;
                out_pkg.data.ap_member(next_vertex_ordered_t, vertex) = weight_pkg.data.range(31, 0);
                out_pkg.data.ap_member(next_vertex_ordered_t, id)
                    = sync_pkg.data.ap_member(burst_sync_t, id);
                out_pkg.last = sync_pkg.last;
                update.write(out_pkg);
            }
        }
        else
        {
            flush_counter ++;
            if (flush_counter > FLUSH_WINDOW)
            {
                break;
            }
        }
    }
}

#endif