#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "sc_weight_factor.h"

#define NUM_OF_WEIGHTS (16)
#define NUM_OF_OS (4)
extern "C" {
    void merge_weight ( next_vertex_stream_t        &in1,
                        next_vertex_stream_t        &in2,
                        next_vertex_stream_t        &in3,
                        next_vertex_stream_t        &in4,
                        weight_stream_t             &weight_out
                      ) {
#pragma HLS interface ap_ctrl_none port=return

        uint32_t out_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=out_data   dim=0

        ap_uint<4> counter = 0;

        next_vertex_pkg_t pkg_1;
        next_vertex_pkg_t pkg_2;
        next_vertex_pkg_t pkg_3;
        next_vertex_pkg_t pkg_4;
        counter = 0;
merge_main: while (1)
        {
            if (in4.read_nb(pkg_4))
            {
                pkg_1 = in1.read();
                pkg_2 = in2.read();
                pkg_3 = in3.read();

                if (pkg_4.last)
                {
                    counter = 0;
                }
                else {
                    uint32_t updated_w_1 = pkg_1.data.ap_member(next_vertex_t, vertex);
                    uint32_t updated_w_2 = pkg_2.data.ap_member(next_vertex_t, vertex);
                    uint32_t updated_w_3 = pkg_3.data.ap_member(next_vertex_t, vertex);
                    uint32_t updated_w_4 = pkg_4.data.ap_member(next_vertex_t, vertex);


                    //if (updated_w == 0xFFFFFFFF )
                    //    updated_w = SC_BACK;
                    //else if ((updated_w & 0x80000000) == 0x80000000 )
                    //    updated_w = 0;

                    out_data[counter + 0] = updated_w_1;
                    out_data[counter + 1] = updated_w_2;
                    out_data[counter + 2] = updated_w_3;
                    out_data[counter + 3] = updated_w_4;

                    if (counter == NUM_OF_WEIGHTS - NUM_OF_OS)
                {
                    weight_item_t       weight_data;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS; i++)
                        {
#pragma HLS UNROLL
                            weight_data.ap_member_array(weight_t, data, i) = out_data[i];
                        }

                        weight_pkg_t  weight_pkg;
                        weight_pkg.data = weight_data;

                        weight_out.write(weight_pkg);
                    }
                    counter += NUM_OF_OS;
                }
            }


        }
    }
}



void merge_weight_2 ( next_vertex_stream_t        &in,
                      weight_stream_t             &weight_out
                    ) {
#pragma HLS interface ap_ctrl_none port=return

    uint32_t out_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=out_data   dim=0
    ap_uint<4> counter = 0;

    next_vertex_pkg_t pkg;
    counter = 0;
merge_main: while (1)
    {
        pkg = in.read();
        {
            if (pkg.last)
            {
                counter = 0;
            }
            else
            {
                uint32_t updated_w = pkg.data.ap_member(next_vertex_t, vertex);
                if ((updated_w & 0x80000000) == 0x80000000 )
                    updated_w = 0;

                out_data[counter] = updated_w;
                if (counter == NUM_OF_WEIGHTS - 1)
                {
                    weight_item_t       weight_data;

pps_done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS; i++)
                    {
#pragma HLS UNROLL
                        weight_data.range(32 * (i + 1) - 1, 32 * i ) = out_data[i];
                    }

                    weight_pkg_t    weight_pkg;
                    weight_pkg.data = weight_data;

                    weight_out.write(weight_pkg);
                }
                counter++;
            }
        }


    }
}