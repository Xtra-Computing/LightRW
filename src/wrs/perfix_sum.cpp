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
    void perfix_sum(        burst_sync_stream_t             &burst_sync,
                            weight_stream_t                 &partial_sum_input,
                            weight_stream_t                 &partial_sum_output,
                            weight_stream_t                 &perfix_sum_output
                   ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            volatile uint8_t flush_counter = 0;
            volatile uint32_t base_sum[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=base_sum   dim=0

init:       for (int i = 0; i < NUM_OF_WEIGHTS; i++) {
#pragma HLS UNROLL
                base_sum[i] = 0;
            }

read: while (1)
            {
                burst_sync_pkg_t  sync_pkg;
                if (burst_sync.read_nb(sync_pkg)) {
                    flush_counter = 0;
                    weight_pkg_t weight_pkg = partial_sum_input.read();
                    volatile uint32_t current_base =  weight_pkg.data.range(512 - 1 , 512 - 32);

                    weight_item_t       w_data = weight_pkg.data;
                    uint32_t            pps_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=pps_data           dim=0
                    uint32_t            pps_data_out[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=pps_data_out       dim=0

                    weight_item_t       w_data_out;

pps_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++) {
#pragma HLS UNROLL
                        pps_data[i] = w_data.range(32 * (i + 1) - 1, 32 * i );
                    }

                    for (int i = 0; i < NUM_OF_WEIGHTS; i++) {
#pragma HLS UNROLL
                        pps_data_out[i] = base_sum[i];
                    }

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++) {
#pragma HLS UNROLL
                        w_data_out.range(32 * (i + 1) - 1, 32 * i ) = pps_data_out[i];
                    }

                    weight_pkg_t out_pkg;
                    out_pkg.data = w_data_out;

                    perfix_sum_output.write(out_pkg);
                    partial_sum_output.write(weight_pkg);



                    if (sync_pkg.data.ap_member(burst_sync_t, burst_id) & 0x8000)
                    {
                        for (int i = 0; i < NUM_OF_WEIGHTS; i++) {
#pragma HLS UNROLL
                            base_sum[i] = 0;
                        }
                    }
                    else
                    {
                        for (int i = 0; i < NUM_OF_WEIGHTS; i++) {
#pragma HLS UNROLL
                            base_sum[i] += current_base;
                        }

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