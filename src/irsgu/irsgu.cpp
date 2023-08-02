#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rng_type.h"
#include "init_state.h"


#define BREAK_NUM           (9)

const rng_state_t init_state_array[BREAK_NUM] = {
    INIT_STATE_0,
    INIT_STATE_1,
    INIT_STATE_2,
    INIT_STATE_3,
    INIT_STATE_4,
    INIT_STATE_5,
    INIT_STATE_6,
    INIT_STATE_7,
    INIT_STATE_8,
};

void ssrng_update(rng_state_inner_stream_t &output, const int id) {
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS FUNCTION_INSTANTIATE  variable=id
    rng_state_t state = init_state_array[id];
    while (1) {
#pragma HLS pipeline II=9
        rng_state_item_t v;
        rng_state_t oldstate = state;
        state = oldstate * RNG_A_SEEKED + RNG_INC_SEEKED;
        v = oldstate;
        output.write(v);
    }
}


void ssrng_merge(
    rng_state_inner_stream_t (&in)[BREAK_NUM],
    rng_state_stream_t &out)
{

#pragma HLS INTERFACE ap_ctrl_none port = return

    while (1) {
#pragma HLS PIPELINE

        rng_state_pkg_t v[BREAK_NUM];
        for (int i = 0; i < BREAK_NUM; i++)
        {
#pragma HLS UNROLL
            v[i].data = in[i].read();
        }

        for (int i = 0; i < BREAK_NUM; i++)
        {
#pragma HLS UNROLL
            out.write(v[i]);
        }
    }
}


extern "C" {
    void irsgu(rng_state_stream_t &out)
    {
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS DATAFLOW
        static rng_state_inner_stream_t s[BREAK_NUM];
#pragma HLS STREAM variable=s  depth=2
        for (int i = 0; i < BREAK_NUM; i++)
        {
#pragma HLS UNROLL
            ssrng_update(s[i], i);
        }
        ssrng_merge(s, out);
    }
}