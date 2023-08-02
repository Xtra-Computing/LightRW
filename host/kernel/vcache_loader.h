#ifndef __VERTEX_LOADER_H__
#define __VERTEX_LOADER_H__

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


class vcache: public cl_krnl
{

public:

    vcache() {}

    vcache(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "vertex_loader", id)
    {

    }


    int init()
    {
        TRACE()

        set_init_done();
        return 0;

    }
};


class vertex_loader: public cl_krnl
{

public:
    vertex_loader() {}

    vertex_loader(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "vcache_mem", id)
    {

    }


    int init(vertex_descriptor_vector_t  &row_index)
    {
        TRACE()
        HOST_BUFFER(row_index, row_index.size() * sizeof(vertex_descriptor_t));
        OCL_CHECK(err, err = krnl.setArg(0, buffer_row_index));

        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_row_index,
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());

        set_init_done();
        return 0;

    }
};




#endif /* __VERTEX_LOADER_H__ */
