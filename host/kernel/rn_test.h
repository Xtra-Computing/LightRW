#ifndef __RN_TEST_H__
#define __RN_TEST_H__

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



class rn_test: public cl_krnl
{
private:
    uint32_t num_of_random_data = 1024 * 1024 * 64;
    uint32_t num_of_res         = 1024;
public:
    uint32_t num_of_access      = 1024 * 1024 * 64;
    uint32_t seed;

    rn_test() {}

    rn_test(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "rn_test", id)
    {
        srand( (unsigned)time(NULL) );

    }


    int init(void)
    {
        std::vector<uint32_t, aligned_allocator<uint32_t> > data_array(num_of_random_data);
        std::vector<uint32_t, aligned_allocator<uint32_t> > res_array(num_of_res);
        for (uint32_t i = 0; i < num_of_random_data; i ++)
        {
            data_array[i] = rand();
        }

        for (uint32_t i = 0; i < num_of_res; i ++)
        {
            res_array[i] = 0;
        }
        seed = rand();

        return mem_transfer(data_array, res_array, num_of_access, seed);

    }
    int mem_transfer(   std::vector<uint32_t, aligned_allocator<uint32_t> >  & data,
                        std::vector<uint32_t, aligned_allocator<uint32_t> > & res,
                        uint32_t num_of_access,
                        uint32_t seed
                    )
    {
        TRACE()
        HOST_BUFFER(data, num_of_random_data * sizeof(uint32_t));
        HOST_BUFFER(res, num_of_res * sizeof(uint32_t));
        uint32_t in_num_of_access = num_of_access;
        uint32_t in_seed = seed;
        OCL_CHECK(err, err = krnl.setArg(0, buffer_data));
        OCL_CHECK(err, err = krnl.setArg(1, buffer_res));
        OCL_CHECK(err, err = krnl.setArg(2, in_num_of_access));
        OCL_CHECK(err, err = krnl.setArg(3, in_seed));
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_data, buffer_res
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());
        set_init_done();
        return 0;
    }
    int exec(   std::vector<uint32_t, aligned_allocator<uint32_t> >  & data,
                std::vector<uint32_t, aligned_allocator<uint32_t> > & res,
                uint32_t num_of_access,
                uint32_t in_seed
            )
    {

        mem_transfer(data, res, num_of_access, in_seed);
        TRACE()

        start = getCurrentTimestamp();
        OCL_CHECK(err, err = mem_q.enqueueTask(krnl));
        OCL_CHECK(err, err = mem_q.finish());
        stop = getCurrentTimestamp();
        printf("exec_time: %lf  thr: %lf M request/s\n",
               (stop - start),
               num_of_access / (stop - start) / 1000 / 1000 );
        return 0;
    }
};

#endif  /* __RN_TEST_H__ */
