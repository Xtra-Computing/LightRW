/**
* Copyright (C) 2020 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

/*******************************************************************************
Description:
    HLS Example using AXI4-master interface for burst read and write
*******************************************************************************/

// Includes
#include <stdio.h>
#include <string.h>

#include "latency_common.h"


// define internal buffer max size
#ifndef BURSTBUFFERSIZE
#error "error BURSTBUFFERSIZE"
#endif

#ifndef OUTSTANDINGSIZE
#error "error OUTSTANDINGSIZE"
#endif




// TRIPCOUNT identifiers
const unsigned int c_size_max = ((BURSTBUFFERSIZE < 2) ? 2 : BURSTBUFFERSIZE);

const unsigned int o_size = OUTSTANDINGSIZE;




extern "C" {
    void latency_test(uint32x16_t *mem, int size,
        stride_addr_stream_t &in,
        dummy_value_stream_t &out) {
// Map pointer a to AXI4-master interface for global memory access
#pragma HLS INTERFACE m_axi port = mem offset = slave bundle = gmem0 num_read_outstanding=o_size max_read_burst_length=c_size_max

// We also need to map a and return to a bundled axilite slave interface
#pragma HLS INTERFACE s_axilite port = size
#pragma HLS INTERFACE s_axilite port = return

        uint32x16_t burstbuffer = 0;

main: for (int i = 0; i < size; i++) {
#pragma HLS PIPELINE off
            stride_addr_pkg_t add_pkg = in.read();
            uint32x1_t addr = add_pkg.data.ap_member(stride_addr_t, addr);
            burstbuffer = mem[addr];
            dummy_value_pkg_t out_pkg;
            out_pkg.data.ap_member(dummy_value_t, value) = burstbuffer.range(31,0);
            out.write(out_pkg);
        }

    }
}
