#ifndef __STABURST_HELPER_H__
#define __STABURST_HELPER_H__


#define BURST_INIT()    burst_cmd_item_t data = pkg.data;                           \
                        ap_uint<1> last = pkg.last;                                 \
                        uint32_t start = data.ap_member(burst_cmd_t, start);        \
                        uint32_t end   = data.ap_member(burst_cmd_t, end);          \
                        uint32_t addr  = data.ap_member(burst_cmd_t, addr);         \
                        uint32_t size  = data.ap_member(burst_cmd_t, size);         \
                        uint32_t state  = data.ap_member(burst_cmd_t, state);       \
                        sync_id_item_t id =  data.ap_member(burst_cmd_t, id);              \
                        sync_id_item_t burst_id = data.ap_member(burst_cmd_t, burst_id);   \
                        if (id & 0x8000){return;}


// inclusive  exclusive
// start      end
inline void burst_cmd_gen(uint32_t addr, uint32_t len,
                          uint32_t start, uint32_t end, sync_id_item_t id,
                          burst_cmd_stream_t &burst_cmd,
                          sync_id_item_t  burst_id,
                          sync_id_item_t  dynamic_id, ap_uint<1> last, uint16_t state)
{
#pragma HLS INLINE
    burst_cmd_item_t bcmd;
    burst_cmd_pkg_t  pkg;
    bcmd.ap_member(burst_cmd_t, addr)       = addr;
    bcmd.ap_member(burst_cmd_t, size)       = len;
    bcmd.ap_member(burst_cmd_t, start)      = start;
    bcmd.ap_member(burst_cmd_t, end)        = end;
    bcmd.ap_member(burst_cmd_t, id)         = id;                   //vertex sync
    bcmd.ap_member(burst_cmd_t, dynamic_id) = dynamic_id;             //dynamic sync
    bcmd.ap_member(burst_cmd_t, burst_id)   = burst_id;             //burst sync
    bcmd.ap_member(burst_cmd_t, state)      = state;             //burst sync

    pkg.data = bcmd;
    pkg.last = last;
    burst_cmd.write(pkg);
}



inline void mask_cmd_gen(uint32_t start, uint32_t end, mask_cmd_stream_t &mask_cmd, ap_uint<1> last)
{
#pragma HLS INLINE
    mask_cmd_item_t mcmd;
    mask_cmd_pkg_t pkg;
    mcmd.ap_member(mask_cmd_t, start) = start;
    mcmd.ap_member(mask_cmd_t, end) = end;
    pkg.data = mcmd;
    pkg.last = last;
    mask_cmd.write(pkg);
}



#endif /* __STABURST_HELPER_H__ */
