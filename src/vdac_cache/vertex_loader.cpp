#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"
#include "stream_operation.h"

#define LOG_VCACHE_SIZE             (16)
#define VCACHE_SIZE                 (1<<LOG_VCACHE_SIZE)

#define V_CACHE_TAG(v)              (v & (VCACHE_SIZE - 1 ))

#define VERTEX_WIDTH                (32)


extern "C" {
    void vertex_loader( next_vertex_stream_t                      &input,
                        //vertex_descriptor_item_t                *vertex,
                        vertex_descriptor_with_state_stream_t     &output,
                        mem_access_cmd_stream_t                   &vertex_cmd,
                        vertex_descriptor_update_stream_t         &cache_update
                      ) {

        ap_uint<64> cache[VCACHE_SIZE];
#pragma HLS BIND_STORAGE variable=cache impl=uram type=ram_s2p

        ap_uint<64> tag[VCACHE_SIZE];
#pragma HLS BIND_STORAGE variable=tag impl=uram type=ram_s2p


        for (int i = 0; i < VCACHE_SIZE; i++)
        {
            tag[i] = 0;
            cache[i] = 0;
        }


check_cache:  while (1) {
#pragma HLS PIPELINE II=1
            next_vertex_pkg_t nv_pkg;
            vertex_descriptor_update_pkg_t cu_pkg;
            if (input.read_nb(nv_pkg)) {

                uint32_t v = nv_pkg.data.ap_member(next_vertex_t, vertex);
                vertex_descriptor_with_state_pkg_t v_pkg;

                uint32_t tag_id = V_CACHE_TAG(v);

                ap_uint<64> tag_value = tag[tag_id];
                vertex_descriptor_item_t vd = cache[tag_id];

                ap_uint<1> mem_access_flag = 0;

                if ((v & 0x80000000) == 0x80000000)
                {
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, start) = v;
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, size)  = 0;
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, state) = 0;
                }
                else if ((tag_value.range(31, 0) ==  v) && (tag_value.range(63, 63) ==  1))
                {
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, start) = vd.ap_member(vertex_descriptor_t, start);
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, size)  = vd.ap_member(vertex_descriptor_t, size);
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, state) = 0;
                }
                else
                {
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, start) = v;  // v is used for update
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, size)  = vd.ap_member(vertex_descriptor_t, size); //size is checked in vcache data
                    v_pkg.data.ap_member(vertex_descriptor_with_state_t, state) = VDS_INVALID;
                    mem_access_flag = 1;
                }


                mem_access_cmd_item_t   cmd_item;
                cmd_item.ap_member(mem_access_cmd_t, addr)  = v; // access in 64-bit

                ap_uint<32> cmd_value = 0;

                if (nv_pkg.last == FLAG_SET)
                    cmd_value |= MM_STATE_MASK_END;

                if (mem_access_flag == FLAG_RESET)
                    cmd_value |= MM_STATE_MASK_DUMMY;

                cmd_item.ap_member(mem_access_cmd_t, state) = cmd_value;

                mem_access_cmd_pkg_t    cmd_pkg;
                cmd_pkg.data = cmd_item;
                cmd_pkg.last = 0;
                vertex_cmd.write(cmd_pkg);


                mem_access_cmd_pkg_t  mem_cmd;
                v_pkg.last = nv_pkg.last;
                output.write(v_pkg);

                if (nv_pkg.last) {
                    break;
                }
            }
            if (cache_update.read_nb(cu_pkg)) {
                uint32_t v = cu_pkg.data.ap_member(vertex_descriptor_update_t, v);
                uint32_t size  = cu_pkg.data.ap_member(vertex_descriptor_update_t, size);
                uint32_t start = cu_pkg.data.ap_member(vertex_descriptor_update_t, start);
                uint32_t tag_id = V_CACHE_TAG(v);

                ap_uint<64> new_tag_value;
                new_tag_value.range(63, 63) = 1;
                new_tag_value.range(31, 0) = v;

                vertex_descriptor_item_t vd;
                vd.ap_member(vertex_descriptor_t, start) = start;
                vd.ap_member(vertex_descriptor_t, size) =  size;
                cache[tag_id] = vd;
                tag[tag_id] = new_tag_value;

            }
        }

        empty_stream(cache_update);
    }
}
