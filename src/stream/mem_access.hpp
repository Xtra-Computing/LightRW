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

#ifndef MEM_BURST_BUFFER_SIZE
#error  "burst size not specified"
#endif

#ifndef MEM_INTERFACE_WIDTH
#error  "interface width not specified"
#endif

#ifndef MEM_KERNEL_NAME
#error  "kernel name not specified"
#endif


#define MEM_FLUSH_WINDOW                (16)



const int burst_size = (MEM_BURST_BUFFER_SIZE < 2) ? (2) : (MEM_BURST_BUFFER_SIZE);

const int burst_ii = MEM_BURST_BUFFER_SIZE;

extern "C" {
    void MEM_KERNEL_NAME(
        const ap_uint<MEM_INTERFACE_WIDTH>          *data,
        mem_access_cmd_stream_t                     &in_cmd,
        mem_access_stream_t(MEM_INTERFACE_WIDTH)    &out_data
    )
    {

#pragma HLS INTERFACE m_axi port = data offset = slave bundle = gmem0 num_read_outstanding=64  max_read_burst_length=burst_size

#pragma HLS INTERFACE s_axilite port = data bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control

        mem_access_item_t(MEM_INTERFACE_WIDTH) buffer[MEM_BURST_BUFFER_SIZE];
#pragma HLS ARRAY_PARTITION variable=buffer   dim=0


flush_pipeline:   while (true)
        {
#pragma HLS PIPELINE off
            volatile uint8_t flush_counter = 0;
            volatile uint8_t notidle = FLAG_RESET;
            mem_access_cmd_pkg_t mem_cmd;

read:       while (1) {
#pragma HLS PIPELINE II=burst_ii rewind

                ap_uint<1> last_flag;
                ap_uint<1> dummy_flag;
                if (in_cmd.read_nb(mem_cmd)) {
                    flush_counter = 0;
                    notidle = FLAG_SET;

                    uint32_t addr = mem_cmd.data.ap_member(mem_access_cmd_t, addr);
                    last_flag  =  ((mem_cmd.data.ap_member(mem_access_cmd_t, state) & MM_STATE_MASK_END)   == MM_STATE_MASK_END);
                    dummy_flag =  ((mem_cmd.data.ap_member(mem_access_cmd_t, state) & MM_STATE_MASK_DUMMY) == MM_STATE_MASK_DUMMY);
burst:      for (int bi = 0; bi < MEM_BURST_BUFFER_SIZE; bi ++)
                    {
#pragma HLS PIPELINE II=burst_size rewind
                        if (dummy_flag == FLAG_SET){
                            buffer[bi] = 0;
                        }
                        else{
                            buffer[bi] = data[addr + bi];
                        }

                        mem_access_pkg_t(MEM_INTERFACE_WIDTH) data_pkg;
                        data_pkg.data = buffer[bi];
                        data_pkg.last =
                            (bi == (MEM_BURST_BUFFER_SIZE - 1)) &&
                            ((mem_cmd.data.ap_member(mem_access_cmd_t, state) & MM_STATE_MASK_END) == MM_STATE_MASK_END);

                        out_data.write(data_pkg);
                    }
                    switch (last_flag)
                    {
                    case 1: return;
                    }
                }
                else
                {
                    if (notidle == FLAG_SET)
                    {
                        flush_counter ++;
                    }
                    if (flush_counter > MEM_FLUSH_WINDOW)
                    {
                        notidle = FLAG_RESET;
                        break;
                    }
                }

            }
        }
    }
}

