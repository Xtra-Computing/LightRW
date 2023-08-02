#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rng_type.h"

#include <ap_int.h>

/*
 * U250  300
 *  1 87.613046 M  87
 *  2 50.074331 M 100
 *  4 30.237670 M 120
 *  8 20.526755 M 160
 * 16 13.010123 M 208
 * 32 7.360042  M 235
 */
#ifndef BURST_BUFFER_SIZE

#error  "burst size not specified"


#endif


const int burst_size = (BURST_BUFFER_SIZE < 2) ? (2) : (BURST_BUFFER_SIZE);

const int burst_ii = BURST_BUFFER_SIZE;

extern "C" {
    void mem_access(
        const ap_uint<512>       *data,
        mem_access_cmd_stream_t  &in_cmd,
        mem_access_data_stream_t &out_data
    )
    {

#pragma HLS INTERFACE m_axi port = data offset = slave bundle = gmem0 num_read_outstanding=64  max_read_burst_length=burst_size

#pragma HLS INTERFACE s_axilite port = data bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

        mem_access_data_item_t buffer[BURST_BUFFER_SIZE];
#pragma HLS ARRAY_PARTITION variable=buffer   dim=0


        while (true)
        {
#pragma HLS PIPELINE II=burst_ii rewind
            ap_uint<1> last_flag;
            mem_access_cmd_pkg_t mem_cmd;
            mem_cmd = in_cmd.read();
            uint32_t addr = mem_cmd.data.ap_member(mem_access_cmd_t, addr);
            last_flag = ((mem_cmd.data.ap_member(mem_access_cmd_t, state) & MM_STATE_MASK_END) == MM_STATE_MASK_END);

burst:      for (int bi = 0; bi < BURST_BUFFER_SIZE; bi ++)
            {
#pragma HLS PIPELINE II=burst_size rewind
                buffer[bi] = data[addr + bi];
                mem_access_data_pkg_t data_pkg;
                data_pkg.data = buffer[bi];
                data_pkg.last =
                    (bi == (BURST_BUFFER_SIZE - 1)) &&
                    ((mem_cmd.data.ap_member(mem_access_cmd_t, state) & MM_STATE_MASK_END) == MM_STATE_MASK_END);

                out_data.write(data_pkg);
            }
            switch (last_flag)
            {
            case 1: return;
            }

        }
    }
}

