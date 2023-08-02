#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>
#include "stream_operation.h"


#include "rng_type.h"


extern "C" {
        void ssink(rng_state_stream_t &input) {
#pragma HLS interface ap_ctrl_none port = return
                while (1) {
#pragma HLS PIPELINE II=1
                        rng_state_pkg_t  data = input.read();
                }
        }
}
