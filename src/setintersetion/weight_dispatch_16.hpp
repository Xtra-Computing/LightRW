#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"

#define NUM_OF_WEIGHTS (16)

extern "C" {
    void weight_dispatch_16 (  weight_stream_t         &weight_in,
                               adjacent_stream_t       &o0,
                               adjacent_stream_t       &o1,
                               adjacent_stream_t       &o2,
                               adjacent_stream_t       &o3,
                               adjacent_stream_t       &o4,
                               adjacent_stream_t       &o5,
                               adjacent_stream_t       &o6,
                               adjacent_stream_t       &o7,
                               adjacent_stream_t       &o8,
                               adjacent_stream_t       &o9,
                               adjacent_stream_t       &o10,
                               adjacent_stream_t       &o11,
                               adjacent_stream_t       &o12,
                               adjacent_stream_t       &o13,
                               adjacent_stream_t       &o14,
                               adjacent_stream_t       &o15
                            ) {
#pragma HLS interface ap_ctrl_none port=return

free_run: while (1)
        {
            weight_pkg_t pkg;

            if (weight_in.read_nb(pkg))
            {
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 0); adj_pkg.last = pkg.last;  o0.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 1); adj_pkg.last = pkg.last;  o1.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 2); adj_pkg.last = pkg.last;  o2.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 3); adj_pkg.last = pkg.last;  o3.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 4); adj_pkg.last = pkg.last;  o4.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 5); adj_pkg.last = pkg.last;  o5.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 6); adj_pkg.last = pkg.last;  o6.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 7); adj_pkg.last = pkg.last;  o7.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 8); adj_pkg.last = pkg.last;  o8.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 9); adj_pkg.last = pkg.last;  o9.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 10); adj_pkg.last = pkg.last; o10.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 11); adj_pkg.last = pkg.last; o11.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 12); adj_pkg.last = pkg.last; o12.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 13); adj_pkg.last = pkg.last; o13.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 14); adj_pkg.last = pkg.last; o14.write(adj_pkg);}
                {adjacent_pkg_t adj_pkg; adj_pkg.data.ap_member(adjacent_t, v) = pkg.data.ap_member_array(weight_t, data, 15); adj_pkg.last = pkg.last; o15.write(adj_pkg);}
            }
        }
    }
}

