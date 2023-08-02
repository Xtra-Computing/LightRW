#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>

#include "ring_manager.h"
#include "step_loader.h"
#include "step_recorder.h"


int main(int argc, char *argv[]) {

    cl_accelerator acc(argc, argv);
    auto q = acc.alloc_task_queue();

    ring_manager    krnl_ring_manager(acc, 0);
    step_loader     krnl_step_loader(acc, 0);
    step_recorder   krnl_step_recorder(acc, 0, 65536);

    krnl_step_loader.set_random_query(1024, 4096, 8);
    krnl_ring_manager.init();
    krnl_step_recorder.init();


    for (int iteration = 0; iteration < 2; iteration ++)
    {
        double start, stop;

        start = getCurrentTimestamp();
        krnl_ring_manager.schedule_task(q);
        krnl_step_recorder.schedule_task(q);
        krnl_step_loader.schedule_task(q);
        q.finish();
        stop = getCurrentTimestamp();

        printf("exec_time: %lf \n",
               (stop - start));
    }
}

