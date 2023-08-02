#ifndef __STEP_COUNTER_H__
#define __STEP_COUNTER_H__

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


class step_counter: public cl_krnl
{
private:

public:
    rw_statistic_vector_t local_statistic;
    cl::Buffer res_buffer;

    step_counter() {}

    step_counter(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "step_counter", id)
    {


    }


    int init(void)
    {
        local_statistic.clear();
        rw_statistic_t  s;
        memset((void *)&s, 0 , sizeof (rw_statistic_t));
        local_statistic.push_back(s);

        return mem_transfer();

    }
    int mem_transfer(void)
    {
        TRACE()
        HOST_BUFFER(local_statistic,  local_statistic.size() * sizeof(rw_statistic_t));

        OCL_CHECK(err, err = krnl.setArg(0, buffer_local_statistic));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_local_statistic
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());
        res_buffer = buffer_local_statistic;
        set_init_done();
        return 0;
    }
    int get_res(void)
    {
        TRACE()

        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({res_buffer}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = mem_q.finish());

        printf("total step: %d \n",
               local_statistic[0].counter);
        return local_statistic[0].counter;
    }
};

#endif /* __STEP_COUNTER_H__ */

