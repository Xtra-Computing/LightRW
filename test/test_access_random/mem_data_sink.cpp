#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rng_type.h"

#include <ap_int.h>


extern "C" {
    void mem_data_sink (
        ap_uint<512>                *res,
        mem_access_data_stream_t    &in_data
    ) {

#pragma HLS INTERFACE m_axi port = res offset = slave bundle = gmem1 // latency = 64

#pragma HLS INTERFACE s_axilite port = res bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

        ap_uint<512> output = 0;

        while (1)
        {
#pragma HLS PIPELINE II=1
            mem_access_data_pkg_t data_pkg = in_data.read();

            output += data_pkg.data;
            if (data_pkg.last == 1)
            {
                break;
            }
        }
        res[0] = output;
    }
}
