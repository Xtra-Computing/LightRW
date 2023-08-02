#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"




extern "C" {
    void vcache_data(   mem_access_stream_t(64)                     &in_data,
                        vertex_descriptor_update_stream_t           &cache_update,
                        vertex_descriptor_with_state_stream_t       &in_vds,
                        vertex_descriptor_stream_t                  &out_vd
                    ) {


#pragma HLS interface ap_ctrl_none port=return


        mem_access_pkg_t(64)  data_pkg;
        volatile uint8_t data_ready_flag = 0;

        vertex_descriptor_with_state_pkg_t vds_pkg;
        volatile uint8_t vds_flag = 0;
        volatile uint8_t passthrough_flag = 0;


out: while(1) {
            data_ready_flag = 0;
            vds_flag = 0;
            passthrough_flag = 0;
merge_main: while (1)
            {

                if (((vds_flag == 0) && (passthrough_flag == 0)) && (in_vds.read_nb(vds_pkg))) {
                    if (vds_pkg.data.ap_member(vertex_descriptor_with_state_t, state) == VDS_INVALID)
                    {
                        vds_flag = 1;
                    }
                    else
                    {
                        passthrough_flag = 1;
                    }
                }

                if ((data_ready_flag == 0) && (in_data.read_nb(data_pkg))) {
                    data_ready_flag = 1;
                }

                if (passthrough_flag == 1)
                {
                    vertex_descriptor_pkg_t  vd_pkg;
                    vd_pkg.data.ap_member(vertex_descriptor_t, start)
                        = vds_pkg.data.ap_member(vertex_descriptor_with_state_t, start);
                    vd_pkg.data.ap_member(vertex_descriptor_t, size)
                        = vds_pkg.data.ap_member(vertex_descriptor_with_state_t, size);

                    vd_pkg.last = vds_pkg.last;

                    out_vd.write(vd_pkg);

                    passthrough_flag = 0;

                    if (vds_pkg.last == 1)
                    {
                        break;
                    }
                }

                else if ((data_ready_flag == 1) && (vds_flag == 1))
                {
                    vertex_descriptor_item_t  new_vd =  data_pkg.data;

                    vertex_descriptor_pkg_t  vd_pkg;
                    vd_pkg.data = data_pkg.data;
                    vd_pkg.last = vds_pkg.last;

                    out_vd.write(vd_pkg);

                    if (new_vd.ap_member(vertex_descriptor_t, size) > vds_pkg.data.ap_member(vertex_descriptor_with_state_t, size))
                    {
                        vertex_descriptor_update_pkg_t update_pkg;
                        update_pkg.data.ap_member(vertex_descriptor_update_t, start) = new_vd.ap_member(vertex_descriptor_t, start);
                        update_pkg.data.ap_member(vertex_descriptor_update_t, size)  = new_vd.ap_member(vertex_descriptor_t, size);
                        update_pkg.data.ap_member(vertex_descriptor_update_t, v)     = vds_pkg.data.ap_member(vertex_descriptor_with_state_t, start);
                        cache_update.write(update_pkg);
                    }

                    data_ready_flag = 0;
                    vds_flag = 0;
                    if (data_pkg.last == 1)
                    {
                        break;
                    }
                }
            }
        }
    }
}
