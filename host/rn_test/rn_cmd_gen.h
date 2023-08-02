#ifndef __RN_CMD_GEN_H__
#define __RN_CMD_GEN_H__

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



class rn_cmd_gen: public cl_krnl
{
public:
    uint32_t num_of_access      = NUM_OF_RN;
    uint32_t seed;

    rn_cmd_gen() {}

    rn_cmd_gen(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "rn_cmd_gen", id)
    {
        srand( (unsigned)time(NULL) );
    }


    int init(void)
    {
        TRACE()
        seed = rand();
        uint32_t in_num_of_access = num_of_access;
        uint32_t in_seed = seed;
        OCL_CHECK(err, err = krnl.setArg(0, in_num_of_access));
        OCL_CHECK(err, err = krnl.setArg(1, in_seed));
        set_init_done();
        return 0;
    }
};

#endif  /* __RN_CMD_GEN_H__ */
