#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#include "dyburst_include/dyburst_helper.h"
#include "dyburst_include/dyburst_const.h"

#define BURSTBUFFERSIZE (DYNAMIC_BURST_MAX_LENGTH)

#define N                           (BURSTBUFFERSIZE)
#define FLUSH_WINDOW                (16)

void dynamic_burst_32(  weight_item_t               *mem,
                        burst_cmd_stream_t          &burst_cmd,
                        burst_sync_stream_t         &burst_sync,
                        weight_stream_t             &weight_output )
{

burst_main: while (1)
    {
        volatile uint8_t flush_counter = 0;
        volatile uint8_t notidle = 0;
read: while (1)
        {
            burst_cmd_pkg_t  pkg;
            if (burst_cmd.read_nb(pkg)) {
                flush_counter = 0;
                notidle = 1;

                BURST_INIT();
                uint16_t dynamic_id = data.ap_member(burst_cmd_t, dynamic_id);

process: for (int j = 0; j < (const unsigned int)N; j++)
                {
#pragma HLS PIPELINE off
                    weight_item_t burstbuffer = 0;
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
                    sync.data.ap_member(burst_sync_t, addr) = addr + (j << 4);
                    sync.data.ap_member(burst_sync_t, id)   = id;


                    if (j == (N - 1))
                    {
                        sync.data.ap_member(burst_sync_t, burst_id)   = burst_id;
                        // one dynamic burst have only one set in the last output
                        sync.data.ap_member(burst_sync_t, dynamic_id) = dynamic_id | 0x8000;
                    }
                    else
                    {
                        // case: only one 32 burst, and it will let all the increment flag in the sync package to be set
                        sync.data.ap_member(burst_sync_t, burst_id)   = burst_id & 0x7fff;
                        sync.data.ap_member(burst_sync_t, dynamic_id) = dynamic_id;
                    }
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
                if (flush_counter > FLUSH_WINDOW)
                {
                    notidle = 0;
                    break;
                }
            }
        }
    }
}


#undef BURSTBUFFERSIZE
#undef N
#undef FLUSH_WINDOW
