#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

extern "C" {
    void update_merge(  next_vertex_ordered_stream_t        &in1,
                        next_vertex_ordered_stream_t        &in2,
                        next_vertex_stream_t                &out )
    {
#pragma HLS interface ap_ctrl_none port=return
        volatile uint16_t current_id = 0;
        ap_uint<1> pkg_1_valid = 0;
        next_vertex_ordered_pkg_t pkg_1;

        ap_uint<1> pkg_2_valid = 0;
        next_vertex_ordered_pkg_t pkg_2;


merge_main: while (1)
        {
            if (pkg_1_valid == 0) {
                if (in1.read_nb(pkg_1)) {
                    pkg_1_valid = 1;
                }
            }

            if (pkg_2_valid == 0) {
                if (in2.read_nb(pkg_2))
                {
                    pkg_2_valid = 1;
                }
            }

            next_vertex_pkg_t out_pkg;
            //next_vertex_pkg_t out_pkg;
            uint16_t tmp_current_id = current_id;
            uint16_t next_id = (tmp_current_id + 1) & 0x7fff;

            if ((pkg_1_valid == 1) && (pkg_1.data.ap_member(next_vertex_ordered_t, id) == tmp_current_id))
            {
                //out_pkg.data = pkg_1.data.ap_member(next_vertex_ordered_t, vertex);
                out_pkg.data = pkg_1.data.ap_member(next_vertex_ordered_t,vertex);
                out_pkg.last = pkg_1.last;
                out.write(out_pkg);
                pkg_1_valid = 0;
                current_id = next_id;
            }
            else if ((pkg_2_valid == 1) && (pkg_2.data.ap_member(next_vertex_ordered_t, id) == tmp_current_id))
            {
                //out_pkg.data = pkg_2.data.ap_member(next_vertex_ordered_t, vertex);
                out_pkg.data = pkg_2.data.ap_member(next_vertex_ordered_t,vertex);;
                out_pkg.last = pkg_2.last;
                out.write(out_pkg);
                pkg_2_valid = 0;
                current_id = next_id;
            }
        }
    }
}
