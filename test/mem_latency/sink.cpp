#include "latency_common.h"

extern "C" {
    void sink(uint32x1_t *out, int size, dummy_value_stream_t &in,time_tag_stream_t &tag_out)
    {
#pragma HLS INTERFACE m_axi port = out offset = slave bundle = gmem0
#pragma HLS INTERFACE s_axilite port = size
#pragma HLS INTERFACE s_axilite port = return
        uint32_t burstbuffer = 0;
sink: for (int i = 0; i < size; i++)
        {
#pragma HLS PIPELINE off
            dummy_value_pkg_t pkg =  in.read();
            time_tag_pkg_t tag_pkg;
            tag_pkg.data.ap_member(time_tag_t, id) = i;
            tag_out.write(tag_pkg);
            burstbuffer += pkg.data.ap_member(dummy_value_t, value);
        }
        out[0] = burstbuffer;
    }
}