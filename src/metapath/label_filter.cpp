#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "metapath_type.h"

#define NUM_OF_WEIGHTS (16)

extern "C" {
    void label_filter ( burst_sync_stream_t         &sync_in,
                        weight_stream_t             &weight_in,
                        burst_sync_stream_t         &sync_out,
                        weight_stream_t             &weight_out
                      ) {
#pragma HLS interface ap_ctrl_none port=return

        burst_sync_pkg_t sync;
        weight_pkg_t    weight;

merge_main: while (1)
        {

            if (sync_in.read_nb(sync) && weight_in.read_in(weight)) {

                weight_item_t       w_data = weight.data;
                uint32_t in_data[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=in_data   dim=0
                uint32_t data_out[NUM_OF_WEIGHTS];
#pragma HLS ARRAY_PARTITION variable=data_out  dim=0
                uint16_t state = sync.data.ap_member(burst_sync_t, state);

init: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                {
#pragma HLS UNROLL
                    in_data[i] = w_data.range(32 * (i + 1) - 1, 32 * i );
                }

cal: for (int i = 0; i < NUM_OF_WEIGHTS; i++)
                {
#pragma HLS UNROLL
                    data_out[i] =  (in_data[i] == state)? 1 : 0;
                }

                weight_item_t       out_weight;

done: for (uint32_t i = 0; i < NUM_OF_WEIGHTS ; i++)
                {
#pragma HLS UNROLL
                    out_weight.range(32 * (i + 1) - 1, 32 * i ) = data_out[i];
                }
                weight_pkg_t       out_weight_pkg;
                out_weight_pkg.data = out_weight;
                sync_out.write(sync);
                weight_out.write(out_weight_pkg);
            }
        }
    }
}
