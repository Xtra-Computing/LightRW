#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "stream_operation.h"



extern "C" {
    void dummy_step_reduce( step_metadata_stream_t     &input,
                            step_metadata_stream_t     &output
                          )
    {
#pragma HLS interface ap_ctrl_none port=return

        while (1)
        {
            step_metadata_pkg_t in_step;
            if (input.read_nb(in_step))
            {
                step_metadata_pkg_t new_step = in_step;
                if (new_step.data.ap_member(step_metadata_t, remain_step) != 0)
                {
                    new_step.data.ap_member(step_metadata_t, remain_step) = (in_step.data.ap_member(step_metadata_t, remain_step) - 1);
                }
                new_step.last = in_step.last;
                output.write(new_step)
            }
        }
    }
}
