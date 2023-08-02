#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#include "staburst_include/staburst_helper.h"
#include "staburst_include/staburst_const.h"

//burst end as stream last

#define SEND_DYNAMIC_CMD(N,S) {                                                         \
    curr_addr += ((const uint32_t)N) * 16;                                              \
    burst_length = ((const uint32_t)N);                                                 \
    ap_uint<1> stream_last = 0;                                                         \
    ap_uint<1> end_of_burst = 0;                                                        \
    if (curr_addr >= end_addr) { stream_last = (1 & pkg.last); end_of_burst = 1;}       \
    else{ stream_last = 0;end_of_burst = 0;}                                            \
    ap_uint<16> dynamic_id = ((dynamic_burst_id & 0x7fff));                             \
    ap_uint<16> burst_id_with_end = burst_id & 0x7fff;                                  \
    burst_id_with_end.range(15,15) = end_of_burst;                                      \
    uint32_t mask_start = ((last < start) ? (start) : (last));                          \
    uint32_t mask_end = ((curr_addr > end) ? (end) : (curr_addr));                      \
    burst_cmd_gen(last,burst_length,mask_start,mask_end,                                \
                 id,S,burst_id_with_end,dynamic_id,stream_last, end_of_burst);          \
}


extern "C" {
    void sta_cmd_generator( vertex_descriptor_ordered_stream_t  &input,
                            burst_cmd_stream_t                  &burst_32_cmd,
                            assert_stream_t                     &assert_in,
                            assert_stream_t                     &assert_out
                         )
    {
#pragma HLS interface ap_ctrl_none port=return
        uint16_t dynamic_burst_id = 0;

cmd_main: while (1)
        {
            assert_pkg_t assert_pkg = assert_in.read();
            vertex_descriptor_ordered_pkg_t pkg = input.read();
            vertex_descriptor_ordered_item_t vd = pkg.data;
            uint32_t start      = vd.ap_member(vertex_descriptor_ordered_t, start);
            uint32_t size       = vd.ap_member(vertex_descriptor_ordered_t, size);
            uint16_t  id        = vd.ap_member(vertex_descriptor_ordered_t, id);
            uint16_t  burst_id  = vd.ap_member(vertex_descriptor_ordered_t, burst_id);

            uint32_t start_addr = start & 0xfffffff0;
            uint32_t end = start + size;
            uint32_t temp_end_addr = end & 0xfffffff0;

            uint32_t end_addr = 0;
            if (temp_end_addr == end)
            {
                end_addr = temp_end_addr;
            }
            else
            {
                end_addr =  temp_end_addr + 16;
            }


            uint32_t curr_addr = start_addr;
cmd_decomposition: while (curr_addr < end_addr)
            {
#pragma HLS PIPELINE style=frp
                uint32_t last = curr_addr;
                uint32_t burst_length;
                //if (end_addr >= (curr_addr + ((const uint32_t)DYNAMIC_BURST_MAX_LENGTH) * 16)) {
                    SEND_DYNAMIC_CMD(DYNAMIC_BURST_MAX_LENGTH, burst_32_cmd)
                    if (assert_pkg.last == 0) {assert_out.write(assert_pkg);};
                //}
                //else {
                //    SEND_DYNAMIC_CMD(1, burst_1_cmd)
                //}

                if (id & 0x8000)
                {

                }
                else
                {
                    dynamic_burst_id ++;
                }

                /*
                mask_cmd_gen(   ((last < start) ? (start) : (last)),
                                ((curr_addr > end) ? (end) : (curr_addr)),
                                mask_cmd,
                                stream_last);
                */
            }


        }
    }
}
