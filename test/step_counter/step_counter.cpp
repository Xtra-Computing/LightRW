#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "test_common.h"

#define BIT_WIDTH   (512)

#define UNIT_SIZE   (128)

extern "C" {
    void step_counter (  ap_uint<BIT_WIDTH>         *mem,
                         step_metadata_stream_t     &in,
                         step_metadata_stream_t     &out

                     ) {
        int counter =  0;
        while (1) {
#pragma HLS PIPELINE II=1
            step_metadata_pkg_t in_step;

            if (in.read_nb(in_step))
            {
                step_metadata_pkg_t out_step;
                out_step.data = in_step.data;
                out_step.last = in_step.last;
                out.write(out_step);

                if (in_step.data.ap_member(step_metadata_t,remain_step)!= 0)
                {
                    counter ++;
                }

                if (in_step.last)
                {
                    break;
                }
            }
        }
        mem[0] = counter;
    }
}
