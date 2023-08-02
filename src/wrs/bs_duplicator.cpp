#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

extern "C" {
    void bs_duplicator (    burst_sync_stream_t             &burst_sync,
                            burst_sync_stream_t             &bs_perfix_sum_input,
                            burst_sync_stream_t             &bs_sample_input
                          ) {
#pragma HLS interface ap_ctrl_none port=return

        for (;;)
        {
            burst_sync_pkg_t in_pkg = burst_sync.read();
            bs_perfix_sum_input.write(in_pkg);
            bs_sample_input.write(in_pkg);
        }
    }
}
