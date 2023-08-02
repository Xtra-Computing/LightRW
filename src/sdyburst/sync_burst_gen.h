#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"

void sync_burst_gen(sync_id_stream_t     &sync_burst_id ) {
#pragma HLS interface ap_ctrl_none port=return
    volatile uint16_t id = 0;
free_run: while (1)
    {
        sync_id_pkg_t pkg;
        pkg.data = id;
        sync_burst_id.write(pkg);
        id = (id + 1) & 0x7fff;
    }
}
