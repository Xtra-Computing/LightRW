#ifndef __STEP_RECORDER_H__
#define __STEP_RECORDER_H__

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>

#include "pcg_basic.h"

#include "helper.h"
#include "host_test_common.h"
#include "fpga_kernel.h"


#include "rw_type.h"


class step_recorder: public cl_krnl
{
private:
    uint32_t num_of_res   = 0;
public:

    uint32_t max_number_of_record = 1024;
    dummy_step_vector_t local_record;

    step_recorder() {}

    step_recorder(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "step_recorder", id)
    {

    }

    step_recorder(cl_accelerator &acc, uint32_t id, uint32_t  max_num)
        : cl_krnl(acc, "step_recorder", id)
    {
        max_number_of_record = max_num;
    }


    int init(void)
    {
        local_record.clear();

        for (uint32_t i = 0; i < max_number_of_record; i ++)
        {
            dummy_step_t r;
            memset((void *)&r, 0 , sizeof (dummy_step_t));
            local_record.push_back(r);
        }

        return mem_transfer();

    }
    int mem_transfer(void)
    {
        TRACE()
        HOST_BUFFER(local_record,  local_record.size() * sizeof(dummy_step_t));

        OCL_CHECK(err, err = krnl.setArg(0, buffer_local_record));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_local_record
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());
        set_init_done();
        return 0;
    }
    int exec(void)
    {
        TRACE()

        start = getCurrentTimestamp();
        OCL_CHECK(err, err = mem_q.enqueueTask(krnl));
        OCL_CHECK(err, err = mem_q.finish());
        stop = getCurrentTimestamp();
        printf("exec_time: %lf \n",
               (stop - start));
        return 0;
    }
};

#endif /* __STEP_RECORDER_H__ */

