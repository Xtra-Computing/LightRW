#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rng_type.h"

#include <ap_int.h>

unsigned int minRand(unsigned int seed, int load) {
    static ap_uint<32> lfsr;

    if (load == 1) lfsr = seed;
    bool b_32 = lfsr.get_bit(32 - 32);
    bool b_22 = lfsr.get_bit(32 - 22);
    bool b_2 = lfsr.get_bit(32 - 2);
    bool b_1 = lfsr.get_bit(32 - 1);
    bool new_bit = b_32 ^ b_22 ^ b_2 ^ b_1;
    lfsr = lfsr >> 1;
    lfsr.set_bit(31, new_bit);

    return lfsr.to_uint();
}

void random_access_cmd_gen( unsigned int in_seed,
                            unsigned int size,
                            mem_access_cmd_inner_stream_t &output
                          )
{
    minRand(in_seed, 1);
    for (int i = 0 ; i < size; i++)
    {
#pragma HLS PIPELINE II=1
        uint32_t seed = minRand(31, 0);
        uint32_t one_read = seed & ((1024u * 512u) - 1u);
        mem_access_cmd_item_t  cmd;
        cmd.ap_member(mem_access_cmd_t, addr)  = one_read;

        if (i == size - 1)
            cmd.ap_member(mem_access_cmd_t, state) = 0x80000000;
        else
            cmd.ap_member(mem_access_cmd_t, state) = 0;

        output.write(cmd);
    }
}



#define BURST_BUFFER_SIZE (2)
void mem_access(
    const ap_uint<512>             *data,
    mem_access_cmd_inner_stream_t  &in_cmd,
    mem_access_data_inner_stream_t &out_data
)
{

    mem_access_data_item_t buffer[BURST_BUFFER_SIZE];
#pragma HLS ARRAY_PARTITION variable=buffer   dim=0

    while (1)
    {
        mem_access_cmd_item_t mem_cmd;
        mem_cmd = in_cmd.read();
        uint32_t addr = mem_cmd.ap_member(mem_access_cmd_t, addr);

        for (int bi = 0; bi < BURST_BUFFER_SIZE; bi ++)
        {
#pragma HLS PIPELINE II=1 rewind
            buffer[bi] = data[addr + bi];
            mem_access_data_item_t data_item;
            data_item = buffer[bi];
            out_data.write(data_item);
        }
        if ((mem_cmd.ap_member(mem_access_cmd_t, state) & 0x80000000) == 0x80000000)
        {
            return;
        }
    }
}

void mem_access(
    ap_uint<512>             *res,

    mem_access_cmd_inner_stream_t  &in_cmd,
    mem_access_data_inner_stream_t &out_data
)
{

    mem_access_data_item_t buffer[BURST_BUFFER_SIZE];
#pragma HLS ARRAY_PARTITION variable=buffer   dim=0

    while (1)
    {
        mem_access_cmd_item_t mem_cmd;
        mem_cmd = in_cmd.read();
        uint32_t addr = mem_cmd.ap_member(mem_access_cmd_t, addr);

        for (int bi = 0; bi < BURST_BUFFER_SIZE; bi ++)
        {
#pragma HLS PIPELINE II=1 rewind
            buffer[bi] = data[addr + bi];
            mem_access_data_item_t data_item;
            data_item = buffer[bi];
            out_data.write(data_item);
        }
        if ((mem_cmd.ap_member(mem_access_cmd_t, state) & 0x80000000) == 0x80000000)
        {
            return;
        }
    }
}

extern "C" {
    void rn_test (  const ap_uint<512> * data,
                    ap_uint<512> * res,
                    const unsigned int size,
                    const unsigned int in_seed
                 ) {

#pragma HLS INTERFACE m_axi port = data offset = slave bundle = gmem0 latency = 300 num_read_outstanding = 64
#pragma HLS INTERFACE m_axi port = res offset = slave bundle = gmem1 // latency = 64

#pragma HLS INTERFACE s_axilite port = data bundle = control
#pragma HLS INTERFACE s_axilite port = res bundle = control

#pragma HLS INTERFACE s_axilite port = in_seed bundle = control
#pragma HLS INTERFACE s_axilite port = size bundle = control
#pragma HLS INTERFACE s_axilite port = return bundle = control



        unsigned int seed = 1;

        ap_uint<512> output = 0;

merge_main:  for (int i = 0; i < size; i++) {
            seed = minRand(31, 0);
            ap_uint<512>  one_read = data[seed & ((1024u * 1024u) - 1u)];
            output += one_read;
        }
        res[0] = output;
    }
}
