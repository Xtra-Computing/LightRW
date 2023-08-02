#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "latency_type.h"
#include "ring_manager.h"

extern "C" {
    void step_cycle (   step_metadata_stream_t     &in,
                        step_metadata_stream_t     &out,
                        time_tag_stream_t          &tag_start,
                        time_tag_stream_t          &tag_end
                    ) {
#pragma HLS interface ap_ctrl_none port=return


merge_main: while (1)
        {
            step_metadata_pkt_t step_pkg;
            if (in.read_nb(step_pkg)) {
                out.write(step_pkg);
                uint32_t id = step_pkg.data.ap_member_offset(step_metadata_t, ring_state, 0, LOG2_RING_SIZE);
                uint32_t remain_step = step_pkg.data.ap_member(step_metadata_t, remain_step);
                if (id == 0)
                {
                    switch (remain_step) {
                    case 4:
                    {
                        time_tag_pkg_t tag_pkg;
                        tag_pkg.data.ap_member(time_tag_t, id) = id;
                        tag_start.write(tag_pkg);
                        break;
                    }
                    case 1:
                    {
                        time_tag_pkg_t tag_pkg;
                        tag_pkg.data.ap_member(time_tag_t, id) = id;
                        tag_end.write(tag_pkg);
                        id ++;
                        break;
                    }
                    }
                }
            }
        }
    }
}
