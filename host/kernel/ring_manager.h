#ifndef __RING_MANAGER_H__
#define __RING_MANAGER_H__

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



class ring_manager: public cl_krnl
{

public:

    ring_manager() {}

    ring_manager(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "ring_manager", id)
    {


    }


    int init(void)
    {
        TRACE();
        uint32_t in_ring_size = 0; // dummy size
        OCL_CHECK(err, err = krnl.setArg(3, in_ring_size));

        set_init_done();

        return 0;

    }
    int exec(void)
    {
        DEBUG_ERROR("can not exec\n")
        return 0;
    }
};

#endif /* __RING_MANAGER_H__ */
