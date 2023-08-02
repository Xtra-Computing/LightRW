#ifndef __MEM_DATA_SINK_H__
#define __MEM_DATA_SINK_H__

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



class mem_data_sink: public cl_krnl
{
private:
    uint32_t num_of_res         = 1024;
public:
    uint32_t num_of_access      = NUM_OF_RN;

    mem_data_sink() {}

    mem_data_sink(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "mem_data_sink", id)
    {

    }


    int init(void)
    {
        TRACE()
        std::vector<uint32_t, aligned_allocator<uint32_t> > res_array(num_of_res);

        for (uint32_t i = 0; i < num_of_res; i ++)
        {
            res_array[i] = 0;
        }

        return mem_transfer(res_array);

    }
    int mem_transfer(std::vector<uint32_t, aligned_allocator<uint32_t> > & res)
    {
        TRACE()
        HOST_BUFFER(res, res.size() * sizeof(uint32_t));
        OCL_CHECK(err, err = krnl.setArg(0, buffer_res));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({  buffer_res
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());
        set_init_done();
        return 0;
    }
};

#endif  /* __MEM_DATA_SINK_H__ */
