#ifndef __TEST_LOAD_H__
#define __TEST_LOAD_H__


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>


#include "helper.h"
#include "host_test_common.h"
#include "fpga_kernel.h"



class test_load: public cl_krnl
{

public:
    uint32_t num_of_step = 0;
    uint32_t seed;

    test_load() {}

    test_load(cl_accelerator &acc, uint32_t cid, uint32_t id)
        : cl_krnl(acc, "test_load",cid, id)
    {
    }

    int mem_transfer(void * p_data, uint32_t size, uint32_t unit_size)
    {
        TRACE()
        HOST_BUFFER_RAW_POINTER(p_data, size);

        uint32_t size_of_transfer = size /unit_size;

        OCL_CHECK(err, err = krnl.setArg(0, size_of_transfer));
        OCL_CHECK(err, err = krnl.setArg(1, buffer_p_data));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_p_data,
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
        return 0;
    }
};

#endif  /*__TEST_LOAD_H__ */
