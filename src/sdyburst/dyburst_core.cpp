#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#include "dyburst_include/dyburst_helper.h"
#include "dyburst_include/dyburst_const.h"

#include "dynamic_burst_1.h"
#include "dynamic_burst_32.h"
#include "single_burst.h"



const unsigned int max_burst32 = DYNAMIC_BURST_MAX_LENGTH;


extern "C" {
    void dyburst_core(   weight_item_t               *mem0,
                         weight_item_t               *mem1,
                         weight_item_t               *mem2,

                         burst_cmd_stream_t          &burst_cmd0,
                         burst_sync_stream_t         &burst_sync0,
                         weight_stream_t             &weight_output0,

                         burst_cmd_stream_t          &burst_cmd1,
                         burst_sync_stream_t         &burst_sync1,
                         weight_stream_t             &weight_output1,

                         burst_cmd_stream_t          &burst_cmd2,
                         burst_sync_stream_t         &burst_sync2,
                         weight_stream_t             &weight_output2 )
    {
#pragma HLS DATAFLOW
#pragma HLS INTERFACE m_axi port = mem0 offset = slave bundle = gmem0 num_read_outstanding=64

#pragma HLS INTERFACE m_axi port = mem1 offset = slave bundle = gmem1 num_read_outstanding=4 max_read_burst_length=max_burst32

#pragma HLS INTERFACE m_axi port = mem2 offset = slave bundle = gmem2 num_read_outstanding=64

        dynamic_burst_1(mem0, burst_cmd0, burst_sync0, weight_output0);
        dynamic_burst_32(mem1, burst_cmd1, burst_sync1, weight_output1);
        single_burst(mem2, burst_cmd2, burst_sync2, weight_output2);

    }
}
