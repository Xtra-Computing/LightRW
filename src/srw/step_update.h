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
#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "stream_operation.h"

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

void step_update(   step_metadata_stream_t              &origin,
                    next_vertex_frp_inner_stream_t      &update,
                    step_metadata_stream_t              &new_query
                )
{

//#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem num_write_outstanding=16

    int count =  0;
    while (1) {
#pragma HLS PIPELINE II=1
        step_metadata_pkg_t origin_step = origin.read();
        next_vertex_frp_item_t  update_vertex_pkg = update.read();


        step_metadata_item_t new_step;
        next_vertex_item_t  nvitem = update_vertex_pkg.ap_frp_data(next_vertex_t);
        uint32_t update_vertex = nvitem.ap_member(next_vertex_t, vertex);

        new_step.ap_member(step_metadata_t, assert) = origin_step.data.ap_member(step_metadata_t, curr_vertex);
        if ((update_vertex & 0x80000000) == 0x80000000)
        {
            new_step.ap_member(step_metadata_t, curr_vertex)
                = (origin_step.data.ap_member(step_metadata_t, curr_vertex)) | 0x80000000;
        }
        else
        {
            count ++;
            new_step.ap_member(step_metadata_t, curr_vertex) = update_vertex;
        }
        new_step.ap_member(step_metadata_t, remain_step)
            = origin_step.data.ap_member(step_metadata_t, remain_step) - 1;

        if (origin_step.data.ap_member(step_metadata_t, remain_step) == 2)
        {
            new_step.ap_member(step_metadata_t, curr_vertex)
                = update_vertex | 0x80000000;
        }

        new_step.ap_member(step_metadata_t, state)
            = origin_step.data.ap_member(step_metadata_t, state);
        new_step.ap_member(step_metadata_t, ring_state)
            = origin_step.data.ap_member(step_metadata_t, ring_state);
        new_step.ap_member(step_metadata_t, inner_state)
            = origin_step.data.ap_member(step_metadata_t, inner_state);
        //mem[counter] = new_step;
        step_metadata_pkg_t new_step_pkg;
        new_step_pkg.data = new_step;
        new_step_pkg.last = update_vertex_pkg.ap_frp_last(next_vertex_t);
        new_query.write(new_step_pkg);

    }

#if 0
    while (1) {
#pragma HLS PIPELINE II=1
        step_metadata_pkg_t pkg = origin.read();
        next_vertex_pkg_t  rng_pkg = update.read();
        if (rng_pkg.last)
        {
            break;
        }
    }



    static step_metadata_inner_stream_t in;
#pragma HLS stream variable=in  depth=2

    static step_metadata_inner_stream_t out;
#pragma HLS stream variable=out  depth=2

    static next_vertex_inner_stream_t new_vertex;
#pragma HLS stream variable=new_vertex  depth=2

#pragma HLS dataflow

    axis_to_hls_stream(origin, in);
    axis_to_hls_stream(update, new_vertex);

    merge_update(in, out, new_vertex, size);

    write_update_back(out, mem, size);
#endif
}

