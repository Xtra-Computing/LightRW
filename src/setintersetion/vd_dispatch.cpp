#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "sc_weight_factor.h"

#define NUM_OF_WEIGHTS (16)
#define FLUSH_WINDOW   (16)

extern "C" {
    void vd_dispatch (  vertex_descriptor_with_state_stream_t   &in,
                        vertex_descriptor_with_state_stream_t   &current,
                        vertex_descriptor_with_state_stream_t   &last
                     ) {
#pragma HLS interface ap_ctrl_none port=return
        uint8_t counter = 0;
        volatile uint32_t size  = 0;  //ad-hoc filter out the access of the adjecent list of last v
        vertex_descriptor_with_state_pkg_t  last_pkg;
free_run: while (1)
        {
            vertex_descriptor_with_state_pkg_t  in_pkg;
            if (in.read_nb(in_pkg))
            {
                if ((counter & 1) == 0){
                    last_pkg = in_pkg;
                }
                else {
                    size = last_pkg.data.ap_member(vertex_descriptor_with_state_t, size);
                    last_pkg.last = in_pkg.last;
                    current.write(last_pkg);

                    if ((size != 0) || (in_pkg.last == 1))
                    {
                        last.write(in_pkg);
                    }
                }
                counter ++;
            }
        }
    }
}

