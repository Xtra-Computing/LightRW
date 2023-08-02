#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "metapath_type.h"


extern "C" {
    void metapath_sm (
                        metapath_sm_lut_item_t      *lut,
                        step_metadata_stream_t      &in,
                        step_metadata_stream_t      &out,
                        metapath_state_stream_t     &state_out
                      ) {
#pragma HLS INTERFACE m_axi port=lut offset=slave bundle=gmem num_read_outstanding=16


#define LUT_SIZE (512)
#define LUT_SIZE_MASK (512 - 1)
        metapath_sm_lut_item_t state_table[LUT_SIZE];
#pragma HLS BIND_STORAGE variable=state_table impl=bram type=ram_2p

        for (int i = 0; i < LUT_SIZE; i++)
        {
            state_table[i] = lut[i];
        }
STATE_UPDATE:  while (1) {
#pragma HLS PIPELINE II=1
            step_metadata_pkg_t v;
            v = in.read();
            ap_uint<9> remain_step = v.data.ap_member_offset(step_metadata_t,remain_step,0,9)
            metapath_sm_lut_item_t updated_state =  state_table[remain_step];

            step_metadata_pkg_t new_v;

            new_v.data.ap_member(step_metadata_t, curr_vertex) = v.data.ap_member(step_metadata_t, curr_vertex);
            new_v.data.ap_member(step_metadata_t, remain_step) = remain_step;
            new_v.data.ap_member(step_metadata_t, state)       = updated_state;
            new_v.data.ap_member(step_metadata_t, ring_state)  = v.data.ap_member(step_metadata_t, ring_state);
            new_v.data.ap_member(step_metadata_t, inner_state) = v.data.ap_member(step_metadata_t, inner_state);
            new_v.last = v.last;

            metapath_state_pkg_t  state_pkg;
            state_pkg.data.ap_member(metapath_state_t,updated_state) = updated_state;
            state_pkg.last = v.last;

            state_out.write(state_pkg);


            out.write(new_v);
            if (v.last) {
                break;
            }
        }
    }
}
