#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"


extern "C" {
    void weight_duplicate_2( weight_stream_t         &weight_in,
                            weight_stream_t         &weight_out1,
                            weight_stream_t         &weight_out2
                          ) {
#pragma HLS interface ap_ctrl_none port=return

        weight_pkg_t pkg;

main: while (1)
        {
            if (weight_in.read_nb(pkg))
            {
                weight_out1.write(pkg);
                weight_out2.write(pkg);
            }

        }
    }
}

