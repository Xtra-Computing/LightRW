#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>


#include "stream_operation.h"
#include "rw_type.h"





extern "C" {
    void vcache_duplicate(  mem_access_stream_t(64)                     &in_data,
                            mem_access_stream_t(64)                     &out_data_1,
                            mem_access_stream_t(64)                     &out_data_2,
                            vertex_descriptor_with_state_stream_t       &in_vds,
                            vertex_descriptor_with_state_stream_t       &out_vds_1,
                            vertex_descriptor_with_state_stream_t       &out_vds_2
                         ) {

#pragma HLS interface ap_ctrl_none port=return
#pragma HLS dataflow

        stream_duplicate_nb< mem_access_pkg_t(64) >(in_data,out_data_1, out_data_2 );
        stream_duplicate_nb< vertex_descriptor_with_state_pkg_t> ( in_vds, out_vds_1,  out_vds_2 );

    }
}