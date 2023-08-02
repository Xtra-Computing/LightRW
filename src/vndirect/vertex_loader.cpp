#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"

#define FLUSH_WINDOW                (16)

extern "C" {
    void vertex_loader( next_vertex_stream_t                    &input,
                        vertex_descriptor_item_t                *vertex,
                        vertex_descriptor_with_state_stream_t   &output
                      ) {
#pragma HLS INTERFACE m_axi port=vertex offset=slave bundle=gmem num_read_outstanding=128


burst_main: while (1)
        {
            volatile uint8_t flush_counter = 0;
            volatile uint8_t notidle = 0;

read:       while (1) {
#pragma HLS PIPELINE II=1
                next_vertex_pkg_t nv_pkg;
                if (input.read_nb(nv_pkg)) {
                    flush_counter = 0;
                    notidle = 1;

                    uint32_t v = nv_pkg.data.ap_member(next_vertex_t, vertex);
                    vertex_descriptor_with_state_pkg_t v_pkg;
                    if ((v & 0x80000000) == 0x80000000)
                    {
                        v_pkg.data.ap_member(vertex_descriptor_with_state_t, start) = v;
                        v_pkg.data.ap_member(vertex_descriptor_with_state_t, size) = 0;
                    }
                    else
                    {
                        vertex_descriptor_item_t vd = vertex[v];
                        v_pkg.data.ap_member(vertex_descriptor_with_state_t, start) = vd.ap_member(vertex_descriptor_t, start);
                        v_pkg.data.ap_member(vertex_descriptor_with_state_t, size) = vd.ap_member(vertex_descriptor_t, size);
                    }

                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, state) =  0;
                    v_pkg.last = nv_pkg.last;

                    output.write(v_pkg);

                    if (nv_pkg.last) {
                        return;
                    }
                }
                else
                {
                    if (notidle == 1)
                    {
                        flush_counter ++;
                    }
                    if (flush_counter > FLUSH_WINDOW)
                    {
                        notidle = 0;
                        break;
                    }
                }
            }

        }
    }
}
