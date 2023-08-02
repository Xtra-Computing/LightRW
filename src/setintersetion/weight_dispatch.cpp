#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"

#define NUM_OF_WEIGHTS (16)
#define NUM_OF_OS  (4)
extern "C" {
    void weight_dispatch (  weight_stream_t         &weight_in,
                            adjacent_stream_t       &adj_out1,
                            adjacent_stream_t       &adj_out2,
                            adjacent_stream_t       &adj_out3,
                            adjacent_stream_t       &adj_out4
                         ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            weight_pkg_t pkg;

            if (weight_in.read_nb(pkg))
            {
                weight_item_t       w_data = pkg.data;
                uint32_t            in_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=in_data   dim=0

in_data_init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS; i++)
                {
#pragma HLS UNROLL
                    in_data[i] = w_data.ap_member_array(weight_t, data, i);
                }

                for (int i = 0; i < NUM_OF_WEIGHTS / NUM_OF_OS; i ++)
                {
#pragma HLS UNROLL II = 4
                    adjacent_pkg_t adj_pkg_1;
                    adj_pkg_1.data.ap_member(adjacent_t, v) = in_data[(i << 2) + 0];
                    if (i == ((NUM_OF_WEIGHTS  / NUM_OF_OS ) - 1))
                        adj_pkg_1.last = pkg.last;
                    else
                        adj_pkg_1.last = 0;
                    adj_out1.write(adj_pkg_1);
                }

                for (int i = 0; i < NUM_OF_WEIGHTS / NUM_OF_OS; i ++)
                {
#pragma HLS UNROLL II = 4
                    adjacent_pkg_t adj_pkg_1;
                    adj_pkg_1.data.ap_member(adjacent_t, v) = in_data[(i << 2) + 1];
                    if (i == ((NUM_OF_WEIGHTS  / NUM_OF_OS ) - 1))
                        adj_pkg_1.last = pkg.last;
                    else
                        adj_pkg_1.last = 0;
                    adj_out2.write(adj_pkg_1);
                }

                for (int i = 0; i < NUM_OF_WEIGHTS / NUM_OF_OS; i ++)
                {
#pragma HLS UNROLL II = 4
                    adjacent_pkg_t adj_pkg_1;
                    adj_pkg_1.data.ap_member(adjacent_t, v) = in_data[(i << 2) + 2];
                    if (i == ((NUM_OF_WEIGHTS  / NUM_OF_OS ) - 1))
                        adj_pkg_1.last = pkg.last;
                    else
                        adj_pkg_1.last = 0;
                    adj_out3.write(adj_pkg_1);
                }


                for (int i = 0; i < NUM_OF_WEIGHTS / NUM_OF_OS; i ++)
                {
#pragma HLS UNROLL II = 4
                    adjacent_pkg_t adj_pkg_1;
                    adj_pkg_1.data.ap_member(adjacent_t, v) = in_data[(i << 2) + 3];
                    if (i == ((NUM_OF_WEIGHTS  / NUM_OF_OS ) - 1))
                        adj_pkg_1.last = pkg.last;
                    else
                        adj_pkg_1.last = 0;
                    adj_out4.write(adj_pkg_1);
                }

            }
        }
    }
}

