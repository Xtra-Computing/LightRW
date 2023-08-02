#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "test_common.h"

#define N_BIS   (32)

extern "C" {
    void test_store_32   (  int                                        size,
                            ap_uint<512>                               *mem,
                            hls::stream< ap_axiu< N_BIS, 0, 0, 0> >    &input

                      ) {
        test_mem_storer< hls::stream< ap_axiu< N_BIS, 0, 0, 0> >,  ap_uint<N_BIS>, ap_axiu< N_BIS, 0, 0, 0>, N_BIS >(size, mem, input);
        //reduce_store<N_BIS>(size, mem, input);
    }
}
