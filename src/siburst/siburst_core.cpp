#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#include "siburst_include/siburst_helper.h"
#include "siburst_include/siburst_const.h"


#define BURSTBUFFERSIZE (DYNAMIC_BURST_MAX_LENGTH)

#define N                           (BURSTBUFFERSIZE)
#define FLUSH_WINDOW                (16)

inline void dynamic_burst_long(  weight_item_t               *mem,
                                 burst_cmd_stream_t          &burst_cmd,
                                 weight_stream_t             &weight_output )
{
#pragma HLS INLINE

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
                if (size == 0)
                {
                    weight_pkg_t  invalid_pkg;
                    weight_item_t invalid_weight ;
                    for (uint8_t k = 0; k < 16; k++)
                    {
#pragma HLS UNROLL
                        invalid_weight.range((32 * (k + 1)) - 1, 32 * k) =  INVALID_VERTEX;
                    }
                    invalid_pkg.data = invalid_weight;
                    invalid_pkg.last = 1;
                    weight_output.write(invalid_pkg);

                }
                else
                {

process: for (int j = 0; j < (const unsigned int)N; j++)
                    {
#pragma HLS PIPELINE off
                        weight_item_t burstbuffer = 0;
                        burstbuffer = mem[(addr >> 4) + j];
                        weight_pkg_t  out_pkg;
                        weight_item_t masked_out = 0;
                        for (uint8_t k = 0; k < 16; k++)
                        {
#pragma HLS UNROLL
                            if (((k + addr) < start) || ((k + addr) >= end))
                                masked_out.range((32 * (k + 1)) - 1, 32 * k) = INVALID_VERTEX;
                            else
                                masked_out.range((32 * (k + 1)) - 1, 32 * k) = burstbuffer.range((32 * (k + 1)) - 1, 32 * k);
                        }
                        out_pkg.data = masked_out;
                        out_pkg.last = state & 0x1;


                        weight_output.write(out_pkg);
                        //burst_sync.write(sync);
                    }
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


const unsigned int max_burst32 = DYNAMIC_BURST_MAX_LENGTH * 2;


extern "C" {
    void siburst_core(   weight_item_t               *mem1,
//                         weight_item_t               *mem2,

//                         burst_cmd_stream_t          &burst_cmd0,
//                         burst_sync_stream_t         &burst_sync0,
//                         weight_stream_t             &weight_output0,

                          burst_cmd_stream_t          &burst_cmd1,
                          weight_stream_t             &weight_output1 )

//                         burst_cmd_stream_t          &burst_cmd2,
//                         burst_sync_stream_t         &burst_sync2,
//                         weight_stream_t             &weight_output2 )
    {
#pragma HLS INTERFACE m_axi port = mem1 offset = slave bundle = gmem1 num_read_outstanding=64 max_read_burst_length=max_burst32

//#pragma HLS INTERFACE m_axi port = mem2 offset = slave bundle = gmem2 num_read_outstanding=64

        //dynamic_burst_1(mem0, burst_cmd0, burst_sync0, weight_output0);
        dynamic_burst_long(mem1, burst_cmd1, weight_output1);
        //single_burst(mem2, burst_cmd2, burst_sync2, weight_output2);

    }
}
