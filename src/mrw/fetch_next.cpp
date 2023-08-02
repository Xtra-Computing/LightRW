#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"

#include "stream_operation.h"

#include "step_update.h"

void fetch_from_mem(sample_index_inner_stream_t  &input,
                    ap_uint<32>                  *vertex,
                    next_vertex_inner_stream_t   &output
                   )
{
outstanding: while (1)
    {
#pragma HLS PIPELINE II=1
        sample_index_item_t address = input.read();
        next_vertex_item_t sample_index;
        sample_index = vertex[address];
        output.write(sample_index);
    }
}


void fetch_next_instance(    sample_index_stream_t          &input,
                             ap_uint<32>                    *vertex,
                             next_vertex_frp_inner_stream_t &output
                        )
{

    while (1)
    {
#pragma HLS PIPELINE II=1
        sample_index_pkg_t in_pkg = input.read();
        uint32_t sample_index = in_pkg.data;
        next_vertex_item_t v;
        if ((sample_index & 0x80000000) == 0x80000000)
        {
            v = sample_index;
        }
        else
        {
            v = vertex[sample_index];
        }
        next_vertex_frp_item_t v_pkg;
        v_pkg.ap_frp_data(next_vertex_t) = v;
        v_pkg.ap_frp_last(next_vertex_t) = in_pkg.last;

        output.write(v_pkg);
        if (in_pkg.last)
        {
            return;
        }

    }

}


extern "C" {
    void fetch_next(    step_metadata_item_t       *res,
                        ap_uint<32>                *vertex,
                        sample_index_stream_t      &input,

                        step_metadata_stream_t     &origin,
                        step_metadata_stream_t     &new_query )
    {
#pragma HLS INTERFACE m_axi port=vertex offset=slave bundle=gmem0  num_read_outstanding=64
#pragma HLS INTERFACE m_axi port=res offset=slave bundle=gmem0

        next_vertex_frp_inner_stream_t       update;
#pragma HLS STREAM variable=update  depth=2047

#pragma HLS DATAFLOW

        fetch_next_instance(input, vertex, update);
        step_update(origin, update, new_query, res);

    }
}




#if 0
static sample_index_inner_stream_t in;
#pragma HLS stream variable=in  depth=64


static next_vertex_inner_stream_t out;
#pragma HLS stream variable=out  depth=64


#pragma HLS dataflow
axis_to_hls_stream(input, in);
fetch_from_mem(in, vertex, out);
hls_to_axis_stream(out, output);



#define INNER_LOOP (3)
ap_uint<32> read_data[INNER_LOOP];
#pragma HLS ARRAY_PARTITION variable = read_data complete dim = 0
ap_uint<1> last_flag[INNER_LOOP];
#pragma HLS ARRAY_PARTITION variable = last_flag complete dim = 0

for (int i = 0; i < INNER_LOOP; i++)
{
    read_data[i] = 0;
    last_flag[i] = 0;
}
int counter = 0;
while (1)
{
    sample_index_pkg_t in_pkg = input.read();
    uint32_t sample_index = in_pkg.data;
    ap_uint<32> data = vertex[sample_index];

    for (int i = INNER_LOOP - 1; i >= 0; i--) {
        if (i == 0) {
            read_data[0] = data;
            last_flag[0] = in_pkg.last;
        } else {
            read_data[i] = read_data[i - 1];
            last_flag[i] = last_flag[i - 1];
        }
    }
    //if (counter == 0)
    {
        vertex_descriptor_pkg_t v;
        output.write(v);
        if (in_pkg.last)
        {
            return;
        }
    }
    //else
    {

    }
}
#endif