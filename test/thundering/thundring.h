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



class test_store: public cl_krnl
{

public:
    uint32_t num_of_data      = 1024 * 1024 * 64;


    test_store() {}

    test_store(cl_accelerator &acc, uint32_t id, uint32_t n)
        : cl_krnl(acc, "test_store_32", id)
    {
        num_of_data = n;
        srand( (unsigned)time(NULL) );

    }


    int init(void)
    {

        std::vector<uint32_t, aligned_allocator<uint32_t> > res_array(num_of_data);

        for (uint32_t i = 0; i < num_of_data; i ++)
        {
            res_array[i] = 0;
        }

        return mem_transfer(res_array);

    }
    int mem_transfer(
                        std::vector<uint32_t, aligned_allocator<uint32_t> > & res
                    )
    {
        TRACE()
        HOST_BUFFER(res, num_of_data * sizeof(uint32_t));
        uint32_t in_num_of_data = num_of_data;
        OCL_CHECK(err, err = krnl.setArg(0, in_num_of_data));
        OCL_CHECK(err, err = krnl.setArg(1, buffer_res));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({ buffer_res
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());
        init_flag = true;
        return 0;
    }
    int exec( void )
    {
        TRACE()
        start = getCurrentTimestamp();
        OCL_CHECK(err, err = mem_q.enqueueTask(krnl));
        TRACE()
        OCL_CHECK(err, err = mem_q.finish());
        TRACE()
        stop = getCurrentTimestamp();
        printf("exec_time: %lf  thr: %lf M request/s\n",
               (stop - start),
               num_of_data / (stop - start) / 1000 / 1000 );
        return 0;
    }
};