#ifndef __NEIGHBOR_MASK_H__
#define __NEIGHBOR_MASK_H__


#define STATS_READ_CURR                 (0)
#define STATS_READ_LAST_WITH_CURR       (1)
#define STATS_END_CURR                  (2)
#define STATS_READ_CURR_WITH_LAST       (3)
#define STATS_END_LAST                  (4)
#define STATS_TMEP                      (5)




#if 0

#define NEIGHBOR_SET  ((((test_counter)<<24) | curr_adj_v) | 0x80000000)
#define NEIGHBOR_UNSET (((test_counter)<<24) | curr_adj_v)

#else

#define NEIGHBOR_SET    ((curr_adj_v == 0)?0:2)
#define NEIGHBOR_UNSET  ((curr_adj_v == 0)?0:4)

#endif


#define READ_LAST() {                                                               \
    last_adj_v_0  = last_adj_pkg.data.ap_member_array(weight_t, data, 0);           \
    last_adj_v_1  = last_adj_pkg.data.ap_member_array(weight_t, data, 1);           \
    last_adj_v_2  = last_adj_pkg.data.ap_member_array(weight_t, data, 2);           \
    last_adj_v_3  = last_adj_pkg.data.ap_member_array(weight_t, data, 3);           \
    last_adj_v_4  = last_adj_pkg.data.ap_member_array(weight_t, data, 4);           \
    last_adj_v_5  = last_adj_pkg.data.ap_member_array(weight_t, data, 5);           \
    last_adj_v_6  = last_adj_pkg.data.ap_member_array(weight_t, data, 6);           \
    last_adj_v_7  = last_adj_pkg.data.ap_member_array(weight_t, data, 7);           \
    last_adj_v_8  = last_adj_pkg.data.ap_member_array(weight_t, data, 8);           \
    last_adj_v_9  = last_adj_pkg.data.ap_member_array(weight_t, data, 9);           \
    last_adj_v_10 = last_adj_pkg.data.ap_member_array(weight_t, data, 10);          \
    last_adj_v_11 = last_adj_pkg.data.ap_member_array(weight_t, data, 11);          \
    last_adj_v_12 = last_adj_pkg.data.ap_member_array(weight_t, data, 12);          \
    last_adj_v_13 = last_adj_pkg.data.ap_member_array(weight_t, data, 13);          \
    last_adj_v_14 = last_adj_pkg.data.ap_member_array(weight_t, data, 14);          \
    last_adj_v_15 = last_adj_pkg.data.ap_member_array(weight_t, data, 15);          \
}

#define  UPDATE_COMPARE_MASK(_emask) {                                                                \
                ap_uint<1>    equal_mask_0;                                                           \
                ap_uint<1>    equal_mask_1;                                                           \
                ap_uint<1>    equal_mask_2;                                                           \
                ap_uint<1>    equal_mask_3;                                                           \
                ap_uint<1>    equal_mask_4;                                                           \
                ap_uint<1>    equal_mask_5;                                                           \
                ap_uint<1>    equal_mask_6;                                                           \
                ap_uint<1>    equal_mask_7;                                                           \
                ap_uint<1>    equal_mask_8;                                                           \
                ap_uint<1>    equal_mask_9;                                                           \
                ap_uint<1>    equal_mask_10;                                                          \
                ap_uint<1>    equal_mask_11;                                                          \
                ap_uint<1>    equal_mask_12;                                                          \
                ap_uint<1>    equal_mask_13;                                                          \
                ap_uint<1>    equal_mask_14;                                                          \
                ap_uint<1>    equal_mask_15;                                                          \
                if (last_adj_v_0  == curr_adj_v) {equal_mask_0  = 1; } else {equal_mask_0  = 0;}      \
                if (last_adj_v_1  == curr_adj_v) {equal_mask_1  = 1; } else {equal_mask_1  = 0;}      \
                if (last_adj_v_2  == curr_adj_v) {equal_mask_2  = 1; } else {equal_mask_2  = 0;}      \
                if (last_adj_v_3  == curr_adj_v) {equal_mask_3  = 1; } else {equal_mask_3  = 0;}      \
                if (last_adj_v_4  == curr_adj_v) {equal_mask_4  = 1; } else {equal_mask_4  = 0;}      \
                if (last_adj_v_5  == curr_adj_v) {equal_mask_5  = 1; } else {equal_mask_5  = 0;}      \
                if (last_adj_v_6  == curr_adj_v) {equal_mask_6  = 1; } else {equal_mask_6  = 0;}      \
                if (last_adj_v_7  == curr_adj_v) {equal_mask_7  = 1; } else {equal_mask_7  = 0;}      \
                if (last_adj_v_8  == curr_adj_v) {equal_mask_8  = 1; } else {equal_mask_8  = 0;}      \
                if (last_adj_v_9  == curr_adj_v) {equal_mask_9  = 1; } else {equal_mask_9  = 0;}      \
                if (last_adj_v_10 == curr_adj_v) {equal_mask_10 = 1; } else {equal_mask_10 = 0;}      \
                if (last_adj_v_11 == curr_adj_v) {equal_mask_11 = 1; } else {equal_mask_11 = 0;}      \
                if (last_adj_v_12 == curr_adj_v) {equal_mask_12 = 1; } else {equal_mask_12 = 0;}      \
                if (last_adj_v_13 == curr_adj_v) {equal_mask_13 = 1; } else {equal_mask_13 = 0;}      \
                if (last_adj_v_14 == curr_adj_v) {equal_mask_14 = 1; } else {equal_mask_14 = 0;}      \
                if (last_adj_v_15 == curr_adj_v) {equal_mask_15 = 1; } else {equal_mask_15 = 0;}      \
                _emask.range(0,0)   = equal_mask_0  & 0x01;                                           \
                _emask.range(1,1)   = equal_mask_1  & 0x01;                                           \
                _emask.range(2,2)   = equal_mask_2  & 0x01;                                           \
                _emask.range(3,3)   = equal_mask_3  & 0x01;                                           \
                _emask.range(4,4)   = equal_mask_4  & 0x01;                                           \
                _emask.range(5,5)   = equal_mask_5  & 0x01;                                           \
                _emask.range(6,6)   = equal_mask_6  & 0x01;                                           \
                _emask.range(7,7)   = equal_mask_7  & 0x01;                                           \
                _emask.range(8,8)   = equal_mask_8  & 0x01;                                           \
                _emask.range(9,9)   = equal_mask_9  & 0x01;                                           \
                _emask.range(10,10) = equal_mask_10 & 0x01;                                           \
                _emask.range(11,11) = equal_mask_11 & 0x01;                                           \
                _emask.range(12,12) = equal_mask_12 & 0x01;                                           \
                _emask.range(13,13) = equal_mask_13 & 0x01;                                           \
                _emask.range(14,14) = equal_mask_14 & 0x01;                                           \
                _emask.range(15,15) = equal_mask_15 & 0x01;                                           \
}



#endif /* __NEIGHBOR_MASK_H__ */

