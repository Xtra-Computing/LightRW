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

#include "vcache_loader.h"

#include "test_load.h"
#include "test_store.h"

#define TEST_V_SIZE             (1024 * 64 * 1024)

__attribute__((aligned(4096))) uint32_t test_vertex[TEST_V_SIZE];

__attribute__((aligned(4096))) vertex_descriptor_t out[TEST_V_SIZE];
__attribute__((aligned(4096))) vertex_descriptor_t golden [TEST_V_SIZE];

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
        //q.curr_vertex = query_array[pcg32_random() % query_array.size()]; //pcg32_random() % vertex_num;
        q.curr_vertex  = pcg32_random() %  16;
        q.remain_step = 10;
        query[i]= q;
    }



    cl_accelerator acc(binaryFile);
    auto q = acc.alloc_task_queue();
    int acc_index = 0;

    vertex_loader   krnl_vertex_loader(acc, acc_index);

    vcache          krnl_vcache(acc, acc_index);
    test_load       krnl_load(acc, 32, acc_index);
    test_store      krnl_store(acc, 64, acc_index);



    krnl_vertex_loader.init(row);
    krnl_vcache.init();


    //krnl_step_loader.set_random_query(1024, 4096, 8);


    for (int i = 0; i < TEST_V_SIZE; i ++)
    {
        vertex_descriptor_t vd, correct;
        memset((void *)&vd, 0, sizeof(vertex_descriptor_t));
        test_vertex[i] = query[(i % query_num)].curr_vertex;
        out[i] = vd;
        correct.start = row[test_vertex[i]].start;
        correct.size  = row[test_vertex[i]].size;
        golden[i] = correct;
    }
    TRACE()


    for (int iteration = 0; iteration < 2; iteration ++)
    {
        double start, stop;

        krnl_load.mem_transfer((void *)test_vertex,TEST_V_SIZE * sizeof(uint32_t), sizeof(uint32_t));
        krnl_store.mem_transfer((void *)out, TEST_V_SIZE * sizeof(vertex_descriptor_t), sizeof(vertex_descriptor_t));

        krnl_vertex_loader.schedule_task(q);
        krnl_vcache.schedule_task(q);
        krnl_store.schedule_task(q);


        start = getCurrentTimestamp();
        krnl_load.schedule_task(q);
        q.finish();
        stop = getCurrentTimestamp();

        krnl_store.get_res();
        int error_count = 0;
        for (int i =0; i < TEST_V_SIZE; i ++)
        {
            vertex_descriptor_t res_vd = out[i];
            vertex_descriptor_t golden_vd = golden[i];
            if ((res_vd.start != golden_vd.start) && (res_vd.size != golden_vd.size))
            {
                printf("%d v-> 0x%08x, get (0x%08x 0x%08x) expect (0x%08x 0x%08x)\n", i, test_vertex[i] ,res_vd.start,res_vd.size, golden_vd.start, golden_vd.size);
                error_count ++;
            }
        }

        printf("exec_time: %lf; thr: %lf; error count %d\n",
               (stop - start), TEST_V_SIZE/(stop - start)/1000/1000,  error_count);
    }
}

