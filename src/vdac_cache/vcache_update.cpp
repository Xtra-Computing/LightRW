#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"




extern "C" {
    void vcache_update(   mem_access_stream_t(64)                     &in_data,
                        vertex_descriptor_with_state_stream_t       &in_vds,
                        vertex_descriptor_update_stream_t           &cache_update
                    ) {


#pragma HLS interface ap_ctrl_none port=return


        mem_access_pkg_t(64)  data_pkg;
        vertex_descriptor_with_state_pkg_t vds_pkg;

out:    while (1)
        {
            if (in_data.read_nb(data_pkg)) {
                vds_pkg = in_vds.read();
                vertex_descriptor_pkg_t  vd_pkg;
                if (vds_pkg.data.ap_member(vertex_descriptor_with_state_t, state) == VDS_INVALID)
                {
                    vertex_descriptor_item_t  new_vd =  data_pkg.data;


                    if (new_vd.ap_member(vertex_descriptor_t, size)
                            > vds_pkg.data.ap_member(vertex_descriptor_with_state_t, size))
                    {
                        vertex_descriptor_update_pkg_t update_pkg;
                        update_pkg.data.ap_member(vertex_descriptor_update_t, start)
                            = new_vd.ap_member(vertex_descriptor_t, start);
                        update_pkg.data.ap_member(vertex_descriptor_update_t, size)
                            = new_vd.ap_member(vertex_descriptor_t, size);
                        update_pkg.data.ap_member(vertex_descriptor_update_t, v)
                            = vds_pkg.data.ap_member(vertex_descriptor_with_state_t, start);
                        update_pkg.last = vds_pkg.last;

                        cache_update.write(update_pkg);
                    }

                }
            }
        }
    }
}
