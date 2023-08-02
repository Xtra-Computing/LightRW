#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>

#include "rn_test.h"

#define  NUM_OF_RN (64* 1024 * 1024)

#include "mem_access.h"
#include "mem_data_sink.h"
#include "rn_cmd_gen.h"

#define NUM_PE (1)

int main(int argc, char *argv[]) {

    cl_accelerator acc(argc, argv);
    auto q = acc.alloc_task_queue();

    mem_access     krnl_mem_access(acc, 0);
    mem_data_sink  krnl_mem_data_sink(acc, 0);
    rn_cmd_gen     krnl_rn_cmd_gen(acc, 0);

    krnl_rn_cmd_gen.init();
    krnl_mem_access.init();
    krnl_mem_data_sink.init();


    for (int iteration = 0; iteration < 2; iteration ++)
    {
        double start, stop;
        start = getCurrentTimestamp();



        krnl_mem_access.schedule_task(q);
        krnl_mem_data_sink.schedule_task(q);

        krnl_rn_cmd_gen.schedule_task(q);



        q.finish();
        stop = getCurrentTimestamp();
        printf("exec_time: %lf  thr: %lf M request/s\n",
               (stop - start),
               NUM_PE * NUM_OF_RN / (stop - start) / 1000 / 1000 );
    }
}



#if 0

int main(int argc, char *argv[]) {

    cl_accelerator acc(argc, argv);
    auto q = acc.alloc_task_queue();

    rn_test   p_rn_test[NUM_PE];

    for (int i = 0; i < NUM_PE ; i++)
    {
        p_rn_test[i] = rn_test(acc, i + 2);
        p_rn_test[i].init();
    }
    for (int iteration = 0; iteration < 2; iteration ++)
    {
        double start, stop;
        start = getCurrentTimestamp();

        for (int i = 0; i < NUM_PE ; i++)
        {
            p_rn_test[i].schedule_task(q);
        }

        q.finish();
        stop = getCurrentTimestamp();
        printf("exec_time: %lf  thr: %lf M request/s\n",
               (stop - start),
               NUM_PE * 1024 * 1024 * 64 / (stop - start) / 1000 / 1000 );
    }
}



#endif
