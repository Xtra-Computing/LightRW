#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

extern "C" {
    void intra_merge(   burst_sync_stream_t         &sync_in1,
                        weight_stream_t             &weight_in1,
                        burst_sync_stream_t         &sync_in2,
                        weight_stream_t             &weight_in2,
                        burst_sync_stream_t         &sync_out,
                        weight_stream_t             &weight_out )
    {
#pragma HLS interface ap_ctrl_none port=return
        volatile uint16_t current_id = 0;
        volatile ap_uint<1> pkg_1_last_flag = 0;
        volatile ap_uint<1> pkg_2_last_flag = 0;
        ap_uint<1> pkg_1_valid = 0;
        burst_sync_pkg_t pkg_1;
        ap_uint<1> pkg_2_valid = 0;
        burst_sync_pkg_t pkg_2;

        weight_pkg_t weight_pkg_1;
        weight_pkg_t weight_pkg_2;
merge_main: while (1)
        {
            if (pkg_1_valid == 0) {
                if (sync_in1.read_nb(pkg_1)) {
                    pkg_1_valid = 1;
                    pkg_1_last_flag = pkg_1.last;
                    weight_pkg_1 = weight_in1.read();
                }
            }
            if (pkg_2_valid == 0) {
                if (sync_in2.read_nb(pkg_2))
                {
                    pkg_2_valid = 1;
                    pkg_2_last_flag = pkg_2.last;
                    weight_pkg_2 = weight_in2.read();
                }
            }
            uint16_t tmp_current_id = current_id;
            uint16_t next_id = ((tmp_current_id + 1) & 0x7fff);

            burst_sync_pkg_t out_sync_pkg;
            if ((pkg_1_valid == 1) && ((pkg_1.data.ap_member(burst_sync_t, dynamic_id) & 0x7fff) == tmp_current_id))
            {
                out_sync_pkg.data = pkg_1.data;
                out_sync_pkg.last = pkg_1_last_flag;
                sync_out.write(out_sync_pkg);
                pkg_1_valid = 0;
                weight_out.write(weight_pkg_1);
                if ( pkg_1.data.ap_member(burst_sync_t, dynamic_id) & 0x8000)
                {
                    current_id = next_id;
                }
            }
            else if ((pkg_2_valid == 1) && ((pkg_2.data.ap_member(burst_sync_t, dynamic_id) & 0x7fff) == tmp_current_id))
            {
                out_sync_pkg.data = pkg_2.data;
                out_sync_pkg.last = pkg_2_last_flag;
                sync_out.write(out_sync_pkg);
                pkg_2_valid = 0;
                weight_out.write(weight_pkg_2);
                if ( pkg_2.data.ap_member(burst_sync_t, dynamic_id) & 0x8000)
                {
                    current_id = next_id;
                }
            }
        }
    }
}
