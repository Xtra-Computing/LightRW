
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"




extern "C" {
    void step_duplicator(   step_metadata_stream_t  &in,
                            next_vertex_stream_t    &query,
                            step_metadata_stream_t  &origin,
                            assert_stream_t         &assert_out1)
    {
#pragma HLS interface ap_ctrl_none port = return
        while (1) {

            step_metadata_pkg_t v;
            v = in.read();
            origin.write(v);

            {
                next_vertex_pkg_t next_v;
                next_v.data.ap_member(next_vertex_t, vertex)
                    = v.data.ap_member(step_metadata_t, curr_vertex);
                next_v.last =  0;
                query.write(next_v);

                assert_pkg_t adata;
                adata.data.ap_member(assert_t, last_vertex)
                    = v.data.ap_member(step_metadata_t, assert);
                adata.data.ap_member(assert_t, curr_vertex)
                    = v.data.ap_member(step_metadata_t, curr_vertex);

                adata.last = 0;
                assert_out1.write(adata);

            }
            {
                next_vertex_pkg_t next_v;
                next_v.data.ap_member(next_vertex_t, vertex)
                    = v.data.ap_member(step_metadata_t, assert);
                next_v.last =  v.last;
                query.write(next_v);

                assert_pkg_t adata;
                adata.data.ap_member(assert_t, last_vertex)
                    = v.data.ap_member(step_metadata_t, assert);
                adata.data.ap_member(assert_t, curr_vertex)
                    = v.data.ap_member(step_metadata_t, curr_vertex);
                adata.last = v.last;
                //assert_out2.write(adata);

            }


        }
    }
}
