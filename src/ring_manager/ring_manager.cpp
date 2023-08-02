#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "stream_operation.h"
#include "ring_manager.h"


#if 0

static void write_update_back(step_metadata_inner_stream_t &input,
                              step_metadata_item_t          *mem,
                              int                          size)
{

    for (int i = 0; i < size; i++)
    {
        step_metadata_item_t data = input.read();
        mem[i] = data;
    }
}

#endif




#define MERGE_KERNEL(in, i) (in[4 * i] | in[4 * i + 1] | in[4 * i + 2] | in[4 * i + 3])


static inline ap_uint<1> merge_flag(ap_uint<1> (&input)[128])
{
    ap_uint<1> level_1[32];
    ap_uint<1> level_2[8];
    ap_uint<1> level_3[2];

    for (int i = 0; i < 32; i ++)
    {
#pragma HLS UNROLL
        level_1[i] = MERGE_KERNEL(input, i);
    }

    for (int i = 0; i < 8; i ++)
    {
#pragma HLS UNROLL
        level_2[i] = MERGE_KERNEL(level_1, i);
    }

    for (int i = 0; i < 2; i ++)
    {
#pragma HLS UNROLL
        level_3[i] = MERGE_KERNEL(level_2, i);
    }
    return level_3[0] | level_3[1];

}



extern "C" {
    void ring_manager(  step_metadata_stream_t     &old_query,
                        step_metadata_stream_t     &new_query,
                        step_metadata_stream_t     &out,
                        int                        ring_size
                     )
    {

//#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem num_write_outstanding=16


        ap_uint<32> counter = 0;
        ap_uint<8>  slot_status[RING_SIZE];
#pragma HLS BIND_STORAGE variable=slot_status type=ram_t2p
#pragma HLS DEPENDENCE variable=slot_status intra WAR false

ring_manager_init: for (int i = 0; i < RING_SIZE; i++)
        {
            slot_status[i]  = 0;
        }



        //ap_uint<8> actived_ring_size = ring_size & (RING_SIZE * 2 - 1);
        ap_uint<1> end_of_query = FLAG_RESET;

        ap_uint<8> ring_counter = 0;


ring_manager_main: while (1) {
#pragma HLS PIPELINE II=1

            step_metadata_pkg_t update;
            step_metadata_pkg_t origin;

            ap_uint<1>              slot_update_flg = FLAG_RESET;
            ap_uint<LOG2_RING_SIZE> slot_update_index;
            ap_uint<8>              slot_update_val;
            //ap_member_offset(step_metadata_t,ring_state,0,LOG2_RING_SIZE)
            //ap_member_offset(step_metadata_t,ring_state,LOG2_RING_SIZE,1)

#if 1
            if (old_query.read_nb(update))
            {
                // old query is finished
                if (update.data.ap_member(step_metadata_t, remain_step) == 0)
                {
                    ap_uint<8> index = update.data.ap_member_offset(step_metadata_t, ring_state, 0, LOG2_RING_SIZE);

                    if (update.last)
                    {
                        break;
                    }
//runtime insertion
                    if (new_query.read_nb(origin))
                    {
                        //if there is a new query, occupy the slot released by the old query
                        step_metadata_pkg_t new_query_pkg = origin;

                        new_query_pkg.data.ap_member_offset(step_metadata_t, ring_state, 0, LOG2_RING_SIZE) = index;
                        new_query_pkg.data.ap_member_offset(step_metadata_t, ring_state, LOG2_RING_SIZE, 1) = origin.last;
                        new_query_pkg.last = 0;

                        out.write(new_query_pkg);
                    }
                    else
                    {
                        //if there is no new query, update the local ring state array
                        slot_update_flg   = FLAG_SET;
                        slot_update_index = index;
                        slot_update_val   = FLAG_RESET;
                    }


                }
                else
                {
                    // save the last flag to ring_state
                    if ((update.data.ap_member(step_metadata_t, remain_step) == 1)
                            && (update.data.ap_member_offset(step_metadata_t, ring_state, LOG2_RING_SIZE, 1) == 1))
                    {
                        update.last = 1;
                    }
                    else
                    {
                        update.last = 0;
                    }
                    out.write(update);
                }
            }

//ring insertion
            step_metadata_pkg_t ring_insert;

            if ((slot_status[ring_counter] == FLAG_RESET))
            {
                //traverse all the local ring state array, if there is new query, occupy the slot
                if (new_query.read_nb(ring_insert))
                {
                    step_metadata_pkg_t new_query_pkg = ring_insert;

                    new_query_pkg.data.ap_member_offset(step_metadata_t, ring_state, 0, LOG2_RING_SIZE) = ring_counter;
                    new_query_pkg.data.ap_member_offset(step_metadata_t, ring_state, LOG2_RING_SIZE, 1) = ring_insert.last;
                    new_query_pkg.last = 0;

                    out.write(new_query_pkg);

                    slot_update_flg   = FLAG_SET;
                    slot_update_index = ring_counter;
                    slot_update_val   = FLAG_SET;
                    ring_counter ++;
                }
                else
                {

                }
            }

#endif

            if ((slot_update_flg == FLAG_SET))
            {
                slot_status[slot_update_index] = slot_update_val;
            }


        }
    }
}
