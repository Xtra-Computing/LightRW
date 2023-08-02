#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"


static const double inv_rand_max = 1.0 / ((double)0xffffffffu + 1.0);


extern "C" {
    void cal_sample_index(  vertex_descriptor_stream_t  &input,
                            rng_output_stream_t         &rng,
                            sample_index_stream_t       &output
                         )
    {
#pragma HLS interface ap_ctrl_none port=return

        while (1) {
#pragma HLS PIPELINE II=1
            vertex_descriptor_pkg_t pkg;

            if (input.read_nb(pkg))
            {
                ap_uint<32> degree = pkg.data.ap_member(vertex_descriptor_t, size);
                ap_uint<32> start  = pkg.data.ap_member(vertex_descriptor_t, start);

                rng_output_pkg_t  rng_pkg = rng.read();
                uint32_t rn = rng_pkg.data;


                uint32_t selected_index =   (uint32_t)(((double)(degree)) * rn * inv_rand_max);


                //uint32_t sampled_index =  (uint32_t) (((double) rn / (0xffffffffu + 1.0)) * (degree + 1))

                ap_uint<32> sampled_index = selected_index + start;
                sample_index_pkg_t index_pkg;
                index_pkg.data = sampled_index;
                index_pkg.last = pkg.last;

                output.write(index_pkg);
            }
        }
    }
}
