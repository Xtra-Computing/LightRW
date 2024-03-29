#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"




extern "C" {
    void vcache_vd(     mem_access_stream_t(64)                     &in_data,
                        vertex_descriptor_with_state_stream_t       &in_vds,
                        vertex_descriptor_stream_t                  &out_vd
                  ) {


#pragma HLS interface ap_ctrl_none port=return

        mem_access_pkg_t(64)  data_pkg;
        vertex_descriptor_with_state_pkg_t vds_pkg;

merge_main: while (1)
        {
            if (in_data.read_nb(data_pkg)) {
                vds_pkg = in_vds.read();
                vertex_descriptor_pkg_t  vd_pkg;
                if (vds_pkg.data.ap_member(vertex_descriptor_with_state_t, state) == VDS_INVALID){
                    vd_pkg.data = data_pkg.data;
                }
                else{
                    vd_pkg.data.ap_member(vertex_descriptor_t, start)
                        = vds_pkg.data.ap_member(vertex_descriptor_with_state_t, start);
                    vd_pkg.data.ap_member(vertex_descriptor_t, size)
                        = vds_pkg.data.ap_member(vertex_descriptor_with_state_t, size);
                }
                vd_pkg.last = vds_pkg.last;

                out_vd.write(vd_pkg);
            }
        }
    }
}