#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>


#include "rng_type.h"
#include "init_state.h"

#include "stream_operation.h"

#include "sou_function.h"

#define NRNG (8)

#define KERNEL_ID  (1 * NRNG  - 1)

#ifndef KERNEL_ID
#error "KERNEL_ID not defined"
#endif




/*

void leaf_state_stream( rng_state_stream_t &in,  rng_state_stream_t &out, rng_output_inner_stream_t &inner_out)
{
#pragma HLS interface ap_ctrl_none port=return
    const int id = KERNEL_ID;
    while (1) {
        rng_state_pkg_t s_pkt = in.read();
        out.write(s_pkt);
        rng_state_t update = multi_stream(s_pkt.data,  54u  + id);
        rng32_t leaf_state = output_function(update);
        inner_out.write(leaf_state);
    }
}

void instance( rng_output_inner_stream_t &s_in, rng_output_stream_t &in,  rng_output_stream_t &out)
{
#pragma HLS interface ap_ctrl_none port=return
    const int id = KERNEL_ID;
    const rng32_t tmp = tmp_xor32_s[id];
    rng32_t sg_state = tmp;
    while (1) {

        rng32_t sg = xorshiftjump(sg_state);
        sg_state = sg;
        rng32_t rng;
        rng_output_pkg_t o_pkg = in.read();
        rng_output_pkg_t v;
        if (s_in.read_nb(rng))
        {
            rng32_t output = (sg ^ rng);
            v.data = application_function(output, o_pkg.data);
            out.write(v);
        }
        else
        {
            out.write(o_pkg);
        }


    }
}

*/


void leaf_state_stream( rng_state_inner_stream_t &in,
                        rng_state_inner_stream_t &out,
                        rng_output_inner_stream_t &inner_out,
                        const int local_id)
{
#pragma HLS interface ap_ctrl_none port=return
#pragma HLS function_instantiate variable=local_id
    const int id = KERNEL_ID + local_id;
    while (1) {
        rng_state_t state = in.read();
        out.write(state);
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
        rng32_t state = s_in.read();
        rng32_t output = (sg ^ state);
        out.write(output);

    }
}

extern "C" {
    void rng_instance(       rng_state_stream_t        &s_in,
                             rng_state_stream_t        &s_out,
                             rng_output_stream_t       &r_out)
    {
#pragma HLS interface ap_ctrl_none port=return


        static rng_state_inner_stream_t s;
#pragma HLS stream variable=s  depth=2

        static rng_state_inner_stream_t s_output;
#pragma HLS stream variable=s_output  depth=2

        static rng_output_inner_stream_t o;
#pragma HLS stream variable=o  depth=2

        static rng_output_inner_stream_t leaf_state;
#pragma HLS stream variable=leaf_state  depth=2

#pragma HLS dataflow

        axis_to_hls_stream(s_in, s);

#pragma HLS dataflow


        leaf_state_stream(s, s_output, leaf_state, 0);
        instance(leaf_state, o, 0);


        hls_to_axis_stream(s_output, s_out);
        hls_to_axis_stream(o, r_out);

    }
}
