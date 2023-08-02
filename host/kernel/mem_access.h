#ifndef __MEM_ACCESS_H__
#define __MEM_ACCESS_H__

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



class mem_access: public cl_krnl
{
private:
    uint32_t num_of_random_data = 1024 * 1024 * 64;
public:
    uint32_t num_of_access      = NUM_OF_RN;
    uint32_t seed;

    mem_access() {}

    mem_access(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "mem_access", id)
    {
        srand( (unsigned)time(NULL) );

    }


    int init(void)
    {
        std::vector<uint32_t, aligned_allocator<uint32_t> > data_array(num_of_random_data);
        for (uint32_t i = 0; i < num_of_random_data; i ++)
        {
            data_array[i] = rand();
        }

        return mem_transfer(data_array);

    }
    int mem_transfer(   std::vector<uint32_t, aligned_allocator<uint32_t> >  & data)
    {
        TRACE()
        HOST_BUFFER(data, num_of_random_data * sizeof(uint32_t));
        OCL_CHECK(err, err = krnl.setArg(0, buffer_data));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_data
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());
        set_init_done();
        return 0;
    }
};

#endif  /* __MEM_ACCESS_H__ */
