#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>


#include "rng_type.h"
#include "init_state.h"

#include "rw_type.h"

#include "stream_operation.h"

#include "sou_function.h"

#define NRNG (16)


#define KERNEL_ID  (1)

#ifndef KERNEL_ID
#error "KERNEL_ID not defined"
#endif


void leaf_state_stream( rng_state_inner_stream_t &in,
                        rng_output_inner_stream_t &inner_out,
                        const int local_id)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS function_instantiate variable=local_id
    const int id = KERNEL_ID + local_id;
    while (1) {
        rng_state_t state = in.read();
        rng_state_t update = multi_stream(state,  54u  + id);
        rng32_t leaf_state = output_function(update);
        inner_out.write(leaf_state);
    }
}

void instance(  rng_output_inner_stream_t &s_in,
                rng_output_inner_stream_t &out,
                const int local_id)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS function_instantiate variable=local_id
    const int id = KERNEL_ID + local_id;
    const rng32_t tmp = tmp_xor32_s[id];
    rng32_t sg_state = tmp;
    while (1) {

        rng32_t sg = xorshiftjump(sg_state);
        sg_state = sg;
        rng32_t rng = s_in.read();

        rng32_t output = (sg ^ rng);
        out.write(output);
    }
}


void state_duplicate( rng_state_inner_stream_t &in,
                      rng_state_inner_stream_t (&out)[NRNG] )
{
#pragma HLS interface ap_ctrl_none port=return

    while (1) {
        rng_state_t state = in.read();
        for (int i = 0; i < NRNG; i++)
        {
#pragma HLS UNROLL
            out[i].write(state);
        }
    }
}


void output_merge(  rng_output_inner_stream_t   (&in)[NRNG],
                    weight_inner_stream_t       &out )
{
#pragma HLS interface ap_ctrl_none port=return
    while (1) {
        rng32_t        rngs[NRNG];
#pragma HLS ARRAY_PARTITION variable=rngs   dim=0

        for (int i = 0; i < NRNG; i++)
        {
#pragma HLS UNROLL
            rngs[i] = in[i].read();
        }

        weight_item_t       w_data_out;
        for (int i = 0; i < NRNG ; i++)
        {
#pragma HLS UNROLL
            w_data_out.range(32 * (i + 1) - 1, 32 * i ) = rngs[i];
        }
        out.write(w_data_out);

    }
}


extern "C" {
    void rng_instance(  rng_state_stream_t     &s_in,
                        weight_stream_t        &rng)
    {
#pragma HLS interface ap_ctrl_none port=return


        static rng_state_inner_stream_t s_innner;
#pragma HLS STREAM variable=s_innner  depth=2

        static rng_state_inner_stream_t s[NRNG];
#pragma HLS STREAM variable=s  depth=2

        static rng_output_inner_stream_t leaf_state[NRNG];
#pragma HLS STREAM variable=leaf_state  depth=2

        static rng_output_inner_stream_t o[NRNG];
#pragma HLS STREAM variable=o  depth=2

        static weight_inner_stream_t w_inner;
#pragma HLS STREAM variable=w_inner  depth=2

#pragma HLS DATAFLOW
        axis_to_hls_stream(s_in, s_innner);
        state_duplicate(s_innner, s);

        for (int i = 0; i < NRNG; i++)
        {
#pragma HLS UNROLL
            leaf_state_stream(s[i], leaf_state[i], i);
        }

        for (int i = 0; i < NRNG; i++)
        {
#pragma HLS UNROLL
            instance(leaf_state[i], o[i], i);
        }
        output_merge(o,w_inner);

        hls_to_axis_stream(w_inner, rng);
    }
}