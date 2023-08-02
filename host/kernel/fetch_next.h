#ifndef __FETCH_NEXT__
#define __FETCH_NEXT__

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


class fetch_next: public cl_krnl
{

public:

    fetch_next() {}

    fetch_next(cl_accelerator &acc, uint32_t id)
        : cl_krnl(acc, "fetch_next", id)
    {

    }



    int init(column_list_vector_t  &column_list)
    {
        TRACE()
        HOST_BUFFER(column_list, column_list.size() * sizeof(column_list_t));
        OCL_CHECK(err, err = krnl.setArg(1, buffer_column_list));

        OCL_CHECK(err, err = mem_q.enqueueMigrateMemObjects({   buffer_column_list,
                                                            }, 0 /*0 means from host*/));
        OCL_CHECK(err, err = mem_q.finish());

        set_init_done();
        return 0;

    }
};

#endif /* __FETCH_NEXT__ */
