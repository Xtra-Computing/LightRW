#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#include "metapath_type.h"

#include "dyburst_include/dyburst_helper.h"
#include "dyburst_include/dyburst_const.h"


extern "C" {
    void vd_scheduler(      sync_id_stream_t                    &sync_id,
                            sync_id_stream_t                    &sync_burst_id,
                            vertex_descriptor_stream_t          &input,
                            metapath_state_stream_t             &state_in,
                            vertex_descriptor_ordered_stream_t  &dynamic_burst_vd,
                            next_vertex_ordered_stream_t        &update,
                            burst_cmd_stream_t                  &single_burst )
    {
//#pragma HLS interface ap_ctrl_none port=return

dispatch_main: while (1)
        {
#pragma HLS PIPELINE style=frp
            vertex_descriptor_pkg_t pkg = input.read();
            vertex_descriptor_item_t vertex = pkg.data;

            metapath_state_pkg_t  state_pkg = state_in.read();
            uint16_t state = state_pkg.data.ap_member(metapath_state_t,updated_state);

            uint32_t start = vertex.ap_member(vertex_descriptor_with_state_t, start);
            uint32_t size = vertex.ap_member(vertex_descriptor_with_state_t, size);

            uint32_t start_addr = start & 0xfffffff0;
            uint32_t end = start + size - 1;
            uint32_t exclusive_end = start + size;
            uint32_t end_addr = end & 0xfffffff0;

            sync_id_pkg_t id_pkg = sync_id.read();
            sync_id_item_t id = id_pkg.data;

            if ((size == 0))
            {
                next_vertex_ordered_pkg_t  nvo_pkg;
                nvo_pkg.data.ap_member(next_vertex_ordered_t, vertex)   = 0x80000000; //TODO: should be source;
                nvo_pkg.data.ap_member(next_vertex_ordered_t, id)       = id;

                nvo_pkg.last = pkg.last;
                update.write(nvo_pkg);
            }
            else
            {
                sync_id_pkg_t burst_id_pkg = sync_burst_id.read();
                sync_id_item_t burst_id = burst_id_pkg.data;
                if (start_addr == end_addr)
                {
                    /* magic */

                    burst_cmd_gen(start_addr, 1, start, exclusive_end, id,
                                  single_burst, burst_id & 0x7fff, 0x8000, pkg.last, state);

                }
                else
                {

                    vertex_descriptor_ordered_pkg_t vdo_pkg;
                    vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, start)      = start;
                    vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, size)       = size;
                    vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, state)      = state;
                    vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, id)         = id;
                    vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, burst_id)   = burst_id & 0x7fff;
                    vdo_pkg.last = pkg.last;
                    dynamic_burst_vd.write(vdo_pkg);

                }
            }
            if (pkg.last)
            {
                break;
            }
        }
        // dummy pack for stoping single burst engine
        burst_cmd_gen(0, 1, 0, 0, 0x8000, single_burst, 0x8000, 0x8000, 1, 0);

        // dummy pack for stoping dynamic burst engine
        vertex_descriptor_ordered_pkg_t vdo_pkg;
        vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, start)      = 0;
        vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, size)       =
            (DYNAMIC_BURST_MAX_LENGTH + 1) * 16;
        vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, state)      = 0;
        vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, id)         = 0x8000;
        vdo_pkg.data.ap_member(vertex_descriptor_ordered_t, burst_id)   = 0x8000;
        vdo_pkg.last = 1;
        dynamic_burst_vd.write(vdo_pkg);

    }
}
