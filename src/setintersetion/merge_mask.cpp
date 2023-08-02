#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"




extern "C" {
    void merge_mask(    adjacent_stream_t       &curr_mask,
                        adjacent_stream_t       &last_mask,
                        adjacent_stream_t       &out )
    {
#pragma HLS interface ap_ctrl_none port=return


        adjacent_pkg_t  last_pkg;
free_run: while (1)
        {
            if (last_mask.read_nb(last_pkg))
            {
                adjacent_pkg_t  curr_pkg = curr_mask.read();
                adjacent_pkg_t  out_pkg;
                if (curr_pkg.data.ap_member(adjacent_t,v) == 0)
                    out_pkg = last_pkg;
                else
                    out_pkg = curr_pkg;
                out_pkg.last = 0;
                out.write(out_pkg);
            }
        }
    }
}
