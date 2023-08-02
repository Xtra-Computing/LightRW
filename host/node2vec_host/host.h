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


#include "ring_manager.h"
#include "step_loader.h"
#include "vertex_loader.h"
#include "fetch_next.h"

#include "step_counter.h"

int main(int argc, char *argv[]) {

    pcg32_srandom(42u, 54u);

#include "load_graph.h"


    vertex_descriptor_vector_t row(graph.num_vertices());
    for (auto i = 0; i < graph.num_vertices(); i++) {
        vertex_descriptor_t v;
        v.start = graph.offset_[i];
        v.size  = graph.degree(i);
        row[i] = v;
    }

    column_list_vector_t  column(graph.num_edges());
    for (auto i = 0; i < graph.num_edges(); i++) {
        column_list_t e;
        e.v = graph.adj_[i];
        column[i] = e;
    }
    //query_num = 512;

    query_metadata_vector_t query(query_num);
    for (size_t i = 0; i < query_num ; i++) {
        query_metadata_t q;
        q.curr_vertex = query_array[pcg32_random() % query_array.size()]; //pcg32_random() % vertex_num;
        q.remain_step = 10;
        query[i]= q;
    }



    cl_accelerator acc(binaryFile);
    auto q = acc.alloc_task_queue();
    int acc_index = 0;
    ring_manager    krnl_ring_manager(acc, acc_index);
    step_loader     krnl_step_loader(acc, acc_index);
    vertex_loader   krnl_vertex_loader(acc, acc_index);
    fetch_next      krnl_fetch_next(acc, acc_index);
    step_counter    krnl_step_counter(acc, acc_index);



    krnl_ring_manager.init();
    krnl_step_counter.init();
    krnl_vertex_loader.init(row);
    krnl_fetch_next.init(column);




    //krnl_step_loader.set_random_query(1024, 4096, 8);
    krnl_step_loader.set_query(query);


    for (int iteration = 0; iteration < 2; iteration ++)
    {
        double start, stop;


        krnl_ring_manager.schedule_task(q);
        krnl_step_counter.schedule_task(q);
        krnl_vertex_loader.schedule_task(q);
        krnl_fetch_next.schedule_task(q);

        start = getCurrentTimestamp();
        krnl_step_loader.schedule_task(q);
        q.finish();
        stop = getCurrentTimestamp();

        int n_step = krnl_step_counter.get_res();

        printf("exec_time: %lf; thr %lf\n",
               (stop - start), n_step/(stop - start)/1000/1000);
    }
}

