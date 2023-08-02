#include "latency_common.h"

extern "C" {
    void mark_start(stride_addr_stream_t &in,
                    stride_addr_stream_t &out,
                    time_tag_stream_t &tag_out)
    {
#pragma HLS interface ap_ctrl_none port=return
        uint8_t id = 0;
        while (1) {
#pragma HLS PIPELINE off
            if(!out.full())
            {
                stride_addr_pkg_t pkg;
                if (in.read_nb(pkg))
                {
                    time_tag_pkg_t tag_pkg;
                    tag_pkg.data.ap_member(time_tag_t, id) = id;
                    tag_out.write(tag_pkg);
                    out.write(pkg);
                    id ++;
                }
            }
        }
    }
}