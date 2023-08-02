#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#define NUM_OF_WEIGHTS (16)
#define FLUSH_WINDOW   (16)

template <int T>
void axis_to_hls_stream_frp( hls::stream< ap_axiu<T, 0, 0, 0> > &in, hls::stream< ap_uint<T> > &out )
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS PIPELINE style=frp

    ap_axiu<T, 0, 0, 0> in_axis_data = in.read();
    ap_uint<T> out_data;
    out_data = in_axis_data.data;
    out.write(out_data);
}


template <int T>
void hls_to_axis_stream_frp( hls::stream < ap_uint < T > > &in, hls::stream< ap_axiu<T, 0, 0, 0> > &out )
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS PIPELINE style=frp

    ap_uint < T >  in_hls_data = in.read();
    ap_axiu<T, 0, 0, 0> out_axis_data;
    out_axis_data.data = in_hls_data;
    out.write(out_axis_data);
}


template <int T>
void axis_to_hls_stream_frp_with_last( hls::stream< ap_axiu<T, 0, 0, 0> > &in, hls::stream < ap_uint < T + 1 > > &out )
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS PIPELINE style=frp

    ap_axiu<T, 0, 0, 0> in_axis_data = in.read();
    ap_uint < T + 1 > out_data;
    out_data.range(T - 1, 0) = in_axis_data.data;
    out_data.range(T, T) = in_axis_data.last;
    out.write(out_data);
}


template <int T>
void hls_to_axis_stream_frp_with_last( hls::stream < ap_uint < T + 1 > > &in, hls::stream< ap_axiu<T, 0, 0, 0> > &out )
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS PIPELINE style=frp

    ap_uint < T + 1 >  in_hls_data = in.read();
    ap_axiu<T, 0, 0, 0> out_axis_data;
    out_axis_data.data = in_hls_data.range(T - 1, 0);
    out_axis_data.last = in_hls_data.range(T , T);
    out.write(out_axis_data);
}


void partial_perfix_sum(weight_inner_stream_t       &w_in,
                        weight_inner_stream_t       &w_out )
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS PIPELINE style=frp

    weight_item_t       w_data = w_in.read();
    uint32_t            pps_data[NUM_OF_WEIGHTS];
pps_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
    {
#pragma HLS UNROLL
        pps_data[i] = w_data.range(32 * (i + 1) - 1, 32 * i );
    }
    uint32_t            pps_data_out[NUM_OF_WEIGHTS];

#pragma HLS ARRAY_PARTITION variable=pps_data   dim=0
#pragma HLS ARRAY_PARTITION variable=pps_data_out  dim=0

    pps_data_out[0] = pps_data[0];
    for (int i = 1; i < NUM_OF_WEIGHTS; i++)
    {
#pragma HLS UNROLL
        pps_data_out[i] = pps_data_out[i - 1] + pps_data[i];
    }

    weight_item_t       w_data_out;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
    {
#pragma HLS UNROLL
        w_data_out.range(32 * (i + 1) - 1, 32 * i ) = pps_data_out[i];
    }
    w_out.write(w_data_out);
}


void weight_duplicator(   weight_inner_stream_t             &in,
                          weight_inner_stream_t             &partial,
                          weight_inner_stream_t             &out)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS PIPELINE style=frp

    weight_item_t data = in.read();
    partial.write(data);
    out.write(data);

}


extern "C" {
    void partial_sum(       weight_stream_t                 &weight_input,
                            weight_stream_t                 &weight_output,
                            weight_stream_t                 &partial_sum_output
                    ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            volatile uint8_t flush_counter = 0;
            volatile uint8_t notidle = 0;

read: while (1)
            {
                weight_pkg_t  weight_pkg;
                if (weight_input.read_nb(weight_pkg)) {
                    flush_counter = 0;
                    notidle = 1;

                    weight_item_t       w_data = weight_pkg.data;
                    uint32_t            pps_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=pps_data   dim=0
                    uint32_t            pps_data_out[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=pps_data_out  dim=0

pps_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                    {
#pragma HLS UNROLL
                        pps_data[i] = w_data.range(32 * (i + 1) - 1, 32 * i );
                    }


                    pps_data_out[0] = pps_data[0];
                    for (int i = 1; i < NUM_OF_WEIGHTS; i++)
                    {
#pragma HLS UNROLL
                        pps_data_out[i] = pps_data_out[i - 1] + pps_data[i];
                    }

                    weight_item_t       partial_sum_data;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                    {
#pragma HLS UNROLL
                        partial_sum_data.range(32 * (i + 1) - 1, 32 * i ) = pps_data_out[i];
                    }
                    weight_pkg_t       partial_sum_pkg;
                    partial_sum_pkg.data = partial_sum_data;

                    weight_output.write(weight_pkg);
                    partial_sum_output.write(partial_sum_pkg);

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


#if 0

// R ~(0,1) < W/ perfix W
// R * perfix W <  W *( 2^32)

extern "C" {
    void partial_sum (      weight_stream_t                 &weight_input,
//                            weight_stream_t                 &weight_output
                            weight_stream_t                 &partial_sum_output
                     ) {
#pragma HLS interface ap_ctrl_none port=return

        weight_inner_stream_t                       w_inner;
#pragma HLS STREAM variable=w_inner depth=3

        weight_inner_stream_t                       w_to_partial;
#pragma HLS STREAM variable=w_to_partial depth=3

        weight_inner_stream_t                       w_to_out;
#pragma HLS STREAM variable=w_to_out depth=3

        weight_inner_stream_t                       w_sum_inner;
#pragma HLS STREAM variable=w_sum_inner depth=3


        for (;;)
        {
#pragma HLS DATAFLOW
            axis_to_hls_stream_frp(weight_input, w_inner);
            //weight_duplicator(w_inner,w_to_partial, w_to_out);
            partial_perfix_sum(w_inner, w_sum_inner);
            hls_to_axis_stream_frp(w_sum_inner, partial_sum_output);
            //hls_to_axis_stream_frp(w_to_out, weight_output);
        }
    }
}

#endif

