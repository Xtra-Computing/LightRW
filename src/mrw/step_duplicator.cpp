#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"




extern "C" {
    void step_duplicator(   step_metadata_stream_t              &in,
                            next_vertex_stream_t                &query,
                            step_metadata_stream_t              &origin
                        )
    {
#pragma HLS interface ap_ctrl_none port = return
        while (1) {

            step_metadata_pkg_t v;
            v = in.read();
            origin.write(v);

            next_vertex_pkg_t next_v;
            next_v.data.ap_member(next_vertex_t, vertex)
                = v.data.ap_member(step_metadata_t, curr_vertex);
            next_v.last = v.last;
            query.write(next_v);

        }
    }
}
