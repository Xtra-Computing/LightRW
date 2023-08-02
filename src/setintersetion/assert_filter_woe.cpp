#include <ap_axi_sdata.h>
#include <ap_int.h>
#include <hls_stream.h>

#include "rw_type.h"
#include "rng_type.h"

#define NUM_OF_WEIGHTS (16)
#define FLUSH_WINDOW   (16)

extern "C" {
    void assert_filter_woe (assert_stream_t                     &assert_in,
                        assert_stream_t                     &assert_out,
                        assert_mask_stream_t                &mask) {
#pragma HLS interface ap_ctrl_none port=return


merge_main: while (1)
        {
            assert_mask_pkg_t  sync_pkg;
            if (mask.read_nb(sync_pkg)) {
                if (sync_pkg.last == 1)
                {
                    assert_pkg_t assert_end;
                    assert_end.data = 0xff;
                    assert_end.last = 1;
                    assert_out.write(assert_end);
                }
                else
                {
                    assert_pkg_t assert = assert_in.read();
                    if (sync_pkg.data == 1)
                        assert_out.write(assert);
                }
            }

        }
    }
}



#if 0
 void assert_filter_woe (assert_stream_t                     &assert_in,
                            assert_stream_t                     &assert_out,
                            assert_mask_stream_t                &mask) {
#pragma HLS interface ap_ctrl_none port=return


        assert_mask_pkg_t sync_pkg;
        volatile uint8_t assert_mask_flag = 0;

        assert_pkg_t assert;
        volatile uint8_t assert_flag = 0;


merge_main: while (1)
        {

            if ((assert_mask_flag == 0)&&(mask.read_nb(sync_pkg))) {
                assert_mask_flag = 1;
            }

            if ((assert_flag == 0)&&(assert_in.read_nb(assert))) {
                assert_flag = 1;
            }

            if ((assert_mask_flag == 1) && (assert_flag == 1))
            {
                if (sync_pkg.data == 1)
                    assert_out.write(assert);

                assert_mask_flag = 0;
                assert_flag = 0;
            }
        }
    }
#endif