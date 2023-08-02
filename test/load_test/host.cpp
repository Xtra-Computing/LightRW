#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>


#include "graph.h"
#include "util/command_parser.h"
#include "log.h"

#include "pcg_basic.h"


#include "test_load.h"
#include "test_store.h"

#define TEST_V_SIZE             (1024 * 64 * 1024)

__attribute__((aligned(4096))) uint32_t in[TEST_V_SIZE];

__attribute__((aligned(4096))) uint32_t out[TEST_V_SIZE];


int main(int argc, char *argv[]) {




    cl_accelerator acc(argc, argv);
    auto q = acc.alloc_task_queue();
    int acc_index = 0;

    test_load       krnl_load(acc, 32, acc_index);
    test_store      krnl_store(acc, 32, acc_index);




    //krnl_step_loader.set_random_query(1024, 4096, 8);


    for (int i = 0; i < TEST_V_SIZE; i ++)
    {
        in[i] = pcg32_random();
        out[i] = 0;
    }
    TRACE()


    for (int iteration = 0; iteration < 2; iteration ++)
    {
        double start, stop;

        krnl_load.mem_transfer((void *)in,TEST_V_SIZE * sizeof(uint32_t), sizeof(uint32_t));
        krnl_store.mem_transfer((void *)out, TEST_V_SIZE * sizeof(uint32_t), sizeof(uint32_t));

        krnl_store.schedule_task(q);


        start = getCurrentTimestamp();
        krnl_load.schedule_task(q);
        q.finish();
        stop = getCurrentTimestamp();

        krnl_store.get_res();
        int error_count = 0;
        for (int i =0; i < TEST_V_SIZE; i ++)
        {
            if (in[i] != out[i])
            {
                printf("%d-> get (0x%08x) expect (0x%08x)\n", i, out[i], in[i]);
                error_count ++;
            }
        }

        printf("exec_time: %lf; thr: %lf; error count %d\n",
               (stop - start), TEST_V_SIZE/(stop - start)/1000/1000,  error_count);
    }
}

