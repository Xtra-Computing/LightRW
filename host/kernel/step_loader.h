#ifndef __STEP_LOADER_H__
#define __STEP_LOADER_H__


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



class step_loader: public cl_krnl
{

public:
    uint32_t num_of_step = 0;
    uint32_t seed;
    query_metadata_vector_t local_query;

    step_loader() {}

    step_loader(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "step_loader", id)
    {
    }

    int set_query(query_metadata_vector_t &query)
    {
        local_query.clear();
        local_query.swap(query);
        return mem_transfer();
    }

    int set_random_query(uint32_t n, uint32_t  vertex_num, uint32_t remain_step)
    {
        local_query.clear();
        for (size_t k = 0; k < n; k++) {
            query_metadata_t q;
            q.curr_vertex = pcg32_random() % vertex_num;
            q.remain_step = remain_step;
            local_query.push_back(q);
        }
        return mem_transfer();

    }
    int mem_transfer(void)
    {
        TRACE()
        HOST_BUFFER(local_query, local_query.size() * sizeof(query_metadata_t));
        uint32_t in_num_of_query = local_query.size();
        OCL_CHECK(err, err = krnl.setArg(0, buffer_local_query));
        OCL_CHECK(err, err = krnl.setArg(2, in_num_of_query));
        OCL_CHECK(err, err = krnl.setArg(3, 10)); //not used
        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_local_query,
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

#endif  /*__STEP_LOADER_H__ */
