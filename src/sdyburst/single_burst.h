#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#include "dyburst_include/dyburst_helper.h"

#define BURSTBUFFERSIZE (1)

void single_burst(      weight_item_t               *mem,
                        burst_cmd_stream_t          &burst_cmd,
                        burst_sync_stream_t         &burst_sync,
                        weight_stream_t             &weight_output )
{
// inclusive  exclusive
// start      end

    weight_item_t readbuf[BURSTBUFFERSIZE] = {0};
    volatile uint8_t notidle = 0;

single_burst_main: while (1)
    {
        volatile uint8_t flush_counter = 0;
read: while (1)
        {
            burst_cmd_pkg_t  pkg;
            if (burst_cmd.read_nb(pkg)) {
                notidle = 1;
                flush_counter = 0;

                BURST_INIT();

process: for (int j = 0; j < BURSTBUFFERSIZE; j++)
                {
#pragma HLS PIPELINE II=1
                    weight_item_t burstbuffer = 0;

                    // log(v*4/64)
                    burstbuffer = mem[(addr >> 4) + j];
                    weight_pkg_t  pkg;
                    weight_item_t masked_out = 0;
                    for (uint8_t k = 0; k < 16; k++)
                    {
#pragma HLS UNROLL
                        if (((k + addr) < start) || ((k + addr) >= end))
                            masked_out.range((32 * (k + 1)) - 1, 32 * k) = 0;
                        else
                            masked_out.range((32 * (k + 1)) - 1, 32 * k) = burstbuffer.range((32 * (k + 1)) - 1, 32 * k);
                    }
                    pkg.data = masked_out;
                    pkg.last = last;
                    weight_output.write(pkg);
                    burst_sync_pkg_t sync;
                    sync.data.ap_member(burst_sync_t, addr)       = addr + (j << 4);
                    sync.data.ap_member(burst_sync_t, id)         = id;
                    sync.data.ap_member(burst_sync_t, burst_id)   = (burst_id | 0x8000);
                    sync.data.ap_member(burst_sync_t, dynamic_id) = 0x8000;
                    sync.data.ap_member(burst_sync_t, state)      = state;
                    sync.last = last;
                    burst_sync.write(sync);
                }
            }
            else
            {
                if (notidle == 1)
                {
                    flush_counter ++;
                }
                if (flush_counter > 16)
                {
                    notidle = 0;
                    break;
                }
            }
        }
    }
}

#undef BURSTBUFFERSIZE