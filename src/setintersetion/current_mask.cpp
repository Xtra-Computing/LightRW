#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#define NUM_OF_WEIGHTS (16)
#define FLUSH_WINDOW   (16)



extern "C" {
    void current_mask(      weight_stream_t                 &weight_input,
                            assert_stream_t                 &assert_in,
                            weight_stream_t                 &weight_output
                     ) {
#pragma HLS interface ap_ctrl_none port=return

        volatile uint8_t weight_flag  = 0;
        weight_pkg_t  weight_pkg;

        volatile uint8_t assert_flag  = 0;
        assert_pkg_t  assert_pkg;
        volatile uint32_t  curr_vertex_vol;

read: while (1)
        {

            if ((weight_flag == 0) && weight_input.read_nb(weight_pkg)) {
                weight_flag = 1;
            }

            if ((assert_flag == 0) && assert_in.read_nb(assert_pkg)) {
                assert_flag = 1;
                curr_vertex_vol = assert_pkg.data.ap_member(assert_t, last_vertex);
            }

            if ((weight_flag == 1) && (assert_flag == 1))
            {
                uint32_t curr_vertex = curr_vertex_vol;

                weight_item_t       w_data = weight_pkg.data;
                uint32_t            in_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=in_data   dim=0

                uint32_t            out_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=out_data  dim=0

pps_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                {
#pragma HLS UNROLL
                    in_data[i] = w_data.ap_member_array(weight_t, data, i);
                }

                for (int i = 0; i < NUM_OF_WEIGHTS; i++)
                {
#pragma HLS UNROLL
                    if (in_data[i] == curr_vertex)
                        out_data[i] = 1; // TODO: q
                    else
                        out_data[i] = 0;
                }
                weight_item_t       out_weight;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                {
#pragma HLS UNROLL
                    out_weight.ap_member_array(weight_t, data, i) = out_data[i];
                }

                weight_pkg_t out_pkg;
                out_pkg.data = out_weight;

                weight_output.write(out_pkg);
                assert_flag = 0;
                weight_flag = 0;

            }
        }
    }

}


#if 0
void current_mask(      weight_stream_t                 &weight_input,
                        assert_stream_t                 &assert_in,
                        weight_stream_t                 &weight_output
                 ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
    {
        volatile uint8_t flush_counter = 0;
        volatile uint8_t notidle = 0;

read: while (1)
        {
            weight_pkg_t  weight_pkg;
            assert_pkg_t  assert_pkg;
            if (weight_input.read_nb(weight_pkg)) {
                flush_counter = 0;
                assert_pkg = assert_in.read();
                notidle = 1;
                uint32_t curr_vertex = assert_pkg.data.ap_member(assert_t, last_vertex);

                weight_item_t       w_data = weight_pkg.data;
                uint32_t            in_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=in_data   dim=0

                uint32_t            out_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=out_data  dim=0

pps_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                {
#pragma HLS UNROLL
                    in_data[i] = w_data.ap_member_array(weight_t, data, i);
                }

                for (int i = 0; i < NUM_OF_WEIGHTS; i++)
                {
#pragma HLS UNROLL
                    if (in_data[i] == curr_vertex)
                        out_data[i] = 1; // TODO: q
                    else
                        out_data[i] = 0;
                }
                weight_item_t       out_weight;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                {
#pragma HLS UNROLL
                    out_weight.ap_member_array(weight_t, data, i) = out_data[i];
                }

                weight_pkg_t out_pkg;
                out_pkg.data = out_weight;

                weight_output.write(out_pkg);
                notidle = 0;
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
#endif