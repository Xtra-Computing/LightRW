#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

extern "C" {
    void inter_merge(   burst_sync_stream_t         &sync_in1,
                        weight_stream_t             &weight_in1,
                        burst_sync_stream_t         &sync_in2,
                        weight_stream_t             &weight_in2,
                        burst_sync_stream_t         &sync_out,
                        weight_stream_t             &weight_out )
    {
#pragma HLS interface ap_ctrl_none port=return
        volatile uint16_t current_id = 0;
        ap_uint<1> pkg_1_valid = 0;
        burst_sync_pkg_t pkg_1;
        ap_uint<1> pkg_2_valid = 0;
        burst_sync_pkg_t pkg_2;
merge_main: while (1)
        {
            if (pkg_1_valid == 0) {
                if (sync_in1.read_nb(pkg_1)) {
                    pkg_1_valid = 1;
                }
            }
            if (pkg_2_valid == 0) {
                if (sync_in2.read_nb(pkg_2))
                {
                    pkg_2_valid = 1;
                }
            }
            uint16_t tmp_current_id = current_id;
            uint16_t next_id = ((tmp_current_id + 1) & 0x7fff);

            burst_sync_pkg_t out_sync_pkg;
            if ((pkg_1_valid == 1) && ((pkg_1.data.ap_member(burst_sync_t, burst_id) & 0x7fff) == tmp_current_id))
            {
                out_sync_pkg.data = pkg_1.data;
                out_sync_pkg.last = pkg_1.last;
                sync_out.write(out_sync_pkg);
                pkg_1_valid = 0;
                weight_pkg_t weight_pkg;
                weight_pkg = weight_in1.read();
                weight_out.write(weight_pkg);
                if ( pkg_1.data.ap_member(burst_sync_t, burst_id) & 0x8000)
                {
                    current_id = next_id;
                }
            }
            else if ((pkg_2_valid == 1) && ((pkg_2.data.ap_member(burst_sync_t, burst_id) & 0x7fff) == tmp_current_id))
            {
                out_sync_pkg.data = pkg_2.data;
                out_sync_pkg.last = pkg_2.last;
                sync_out.write(out_sync_pkg);
                pkg_2_valid = 0;
                weight_pkg_t weight_pkg;
                weight_pkg = weight_in2.read();
                weight_out.write(weight_pkg);
                if ( pkg_2.data.ap_member(burst_sync_t, burst_id) & 0x8000)
                {
                    current_id = next_id;
                }
            }
        }
    }
}
