#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"


#include "neighbor_mask.h"


#define NUM_OF_WEIGHTS (16)

#define COMPARISON(emask) \
                        if (emask != 0 )                                                    \
                        {                                                                   \
                            if (curr_adj_v == last_adj_v_15)                                \
                                update_state = STATS_READ_CURR;                             \
                            else                                                            \
                                update_state = STATS_READ_CURR_WITH_LAST;                   \
                            write_flag = 1; out_v = NEIGHBOR_SET;                           \
                            write_last = 0;                                                 \
                        }                                                                   \
                        else if (curr_adj_v < last_adj_v_15 )                               \
                        {                                                                   \
                            update_state = STATS_READ_CURR_WITH_LAST;                       \
                            write_flag = 1; out_v = NEIGHBOR_UNSET;                         \
                            write_last = 0;                                                 \
                        }                                                                   \
                        else                                                                \
                        {                                                                   \
                            update_state = STATS_READ_LAST_WITH_CURR;                       \
                            write_flag = 0; write_last = 0;                                 \
                        }


extern "C" {
    void neighbor_mask(     adjacent_stream_t       &curr_adj_in,
                            weight_stream_t         &last_adj_in,
                            adjacent_stream_t       &out )
    {
#pragma HLS interface ap_ctrl_none port=return

        volatile uint8_t update_state = STATS_READ_CURR;


        volatile uint32_t         last_adj_v_0;
        volatile uint32_t         last_adj_v_1;
        volatile uint32_t         last_adj_v_2;
        volatile uint32_t         last_adj_v_3;
        volatile uint32_t         last_adj_v_4;
        volatile uint32_t         last_adj_v_5;
        volatile uint32_t         last_adj_v_6;
        volatile uint32_t         last_adj_v_7;
        volatile uint32_t         last_adj_v_8;
        volatile uint32_t         last_adj_v_9;
        volatile uint32_t         last_adj_v_10;
        volatile uint32_t         last_adj_v_11;
        volatile uint32_t         last_adj_v_12;
        volatile uint32_t         last_adj_v_13;
        volatile uint32_t         last_adj_v_14;
        volatile uint32_t         last_adj_v_15;

        volatile uint32_t curr_adj_v;

        volatile uint8_t test_counter = 0;

        volatile uint8_t final_current = 0;

        volatile uint8_t write_flag = 0;
        volatile uint8_t write_last = 0;
        volatile uint32_t out_v = 0;


free_run: while (1) {
#pragma HLS  LATENCY max=2 min=2

            switch (update_state)
            {
            case STATS_READ_CURR:
            {
                adjacent_pkg_t  curr_adj_pkg;
                if (curr_adj_in.read_nb(curr_adj_pkg))
                {
                    curr_adj_v = curr_adj_pkg.data.ap_member(adjacent_t, v);
                    if (curr_adj_pkg.last != 1)
                    {
                        update_state = STATS_READ_LAST_WITH_CURR;
                    }
                    else
                    {
                        final_current = 0;
                        update_state = STATS_END_CURR;
                    }

                }
                write_flag = 0; write_last = 0;
                break;
            }

            case STATS_READ_LAST_WITH_CURR:
            {
                weight_pkg_t    last_adj_pkg;
                if (last_adj_in.read_nb(last_adj_pkg))
                {
                    READ_LAST()

                    ap_uint<16>  mask;
                    UPDATE_COMPARE_MASK(mask);

                    if (last_adj_pkg.last != 1)
                    {
                        COMPARISON(mask);
                    }
                    else
                    {
                        if (mask != 0 )
                        {
                            write_flag = 1; out_v = NEIGHBOR_SET;
                        }
                        else
                        {
                            write_flag = 1; out_v = NEIGHBOR_UNSET;
                        }
                        update_state = STATS_END_LAST;
                        write_last = 0;
                        //update_state = STATS_TMEP;
                    }
                }
                else
                {
                    write_flag = 0; write_last = 0;
                }
                break;
            }

            case STATS_READ_CURR_WITH_LAST:
            {
                adjacent_pkg_t  curr_adj_pkg;
                if (curr_adj_in.read_nb(curr_adj_pkg))
                {
                    curr_adj_v = curr_adj_pkg.data.ap_member(adjacent_t, v);
                    ap_uint<16>  mask;
                    UPDATE_COMPARE_MASK(mask);
                    if (curr_adj_pkg.last != 1)
                    {
                        COMPARISON(mask);
                    }
                    else
                    {
                        if (mask != 0 )
                        {
                            final_current = 1;
                        }
                        else
                        {
                            final_current = 0;
                        }
                        update_state = STATS_END_CURR;
                        write_flag = 0; write_last = 0;
                    }

                }
                else {
                    write_flag = 0; write_last = 0;
                }
                break;
            }

            case STATS_END_LAST:
            {

                adjacent_pkg_t  curr_adj_pkg;
                if (curr_adj_in.read_nb(curr_adj_pkg))
                {
                    curr_adj_v = curr_adj_pkg.data.ap_member(adjacent_t, v);
                    ap_uint<16>  mask;
                    UPDATE_COMPARE_MASK(mask);

                    if (mask != 0 )
                    {
                        write_flag = 1; out_v = NEIGHBOR_SET;
                    }
                    else
                    {
                        write_flag = 1; out_v = NEIGHBOR_UNSET;
                    }

                    if (curr_adj_pkg.last == 1)
                    {
                        write_last = 1;
                        update_state = STATS_READ_CURR;
                    }
                    else
                    {
                        write_last = 0;
                    }
                }
                else
                {
                    write_flag = 0; write_last = 0;
                }
                break;
            }
            case STATS_END_CURR:
            {
                weight_pkg_t    last_adj_pkg;
                if (last_adj_in.read_nb(last_adj_pkg))
                {
                    READ_LAST()
                    ap_uint<16>  mask;
                    UPDATE_COMPARE_MASK(mask);

                    if (last_adj_pkg.last == 1)
                    {
                        if ((final_current == 1) || (mask!= 0))
                        {
                            write_flag = 1; out_v = NEIGHBOR_SET;
                        }
                        else
                        {
                            write_flag = 1; out_v = NEIGHBOR_UNSET;
                        }
                        write_last = 1;
                        final_current = 0;
                        update_state = STATS_READ_CURR;
                    }
                    else
                    {
                        if (mask != 0 )
                        {
                            final_current = 1;
                        }
                        else
                        {
                            final_current = 0;
                        }
                        write_flag = 0; write_last = 0;
                    }
                }
                else
                {
                    write_flag = 0; write_last = 0;
                }
                break;
            }
            }
            if (write_flag == 1)
            {
                adjacent_pkg_t  out_pkg;
                out_pkg.data.ap_member(adjacent_t, v) = out_v;
                out_pkg.last = write_last;
                test_counter ++;
                out.write(out_pkg);
            }

        }
    }
}
