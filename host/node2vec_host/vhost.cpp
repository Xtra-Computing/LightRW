#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <xcl2.hpp>
#include "CL/cl_ext_xilinx.h"
#include "graph.h"
#include "rw_data_struct.h"

#include "helper.h"
#include "pcg_basic.h"
#include "accelerator.h"


#include "util/command_parser.h"
#include "log.h"

//#define TEST_V (4011)
#define TEST_V (8)

#define TEST_Q (4096)

#define TEST_QD (80)

#define DISTRIBUTION_CHECK (0)


extern CL_API_ENTRY cl_int CL_API_CALL
xclGetMemObjDeviceAddress(cl_mem mem,
                          cl_device_id device,
                          size_t sz,
                          void* address);


#ifndef  ACC_NUM
#warning "ACC_NUM is set to one"
#define ACC_NUM (1)
#endif
const char * free_run_kernel_list[] = {

};

uint16_t metapath_lut[] = {1, 1, 5, 4, 3, 2, 1, 1}; // label can not be 0

const size_t max_weight_value = 5;




int main(int argc, char** argv) {
    double start = getCurrentTimestamp();

#include "accinit.h"




    double fpga_start = getCurrentTimestamp();

    accelerator acc[ACC_NUM];

    for (int i = 0; i < ACC_NUM; i++)
    {
        //acc[i].set_static_burst_debug(FLAG_SET);
        acc[i].init_acc(device, context, cprogram, i, FLAG_SET);

        //acc[i].init_label_filter(metapath_lut, ARRAY_SIZE(metapath_lut));

    }
    double memcpy_start = getCurrentTimestamp();
    for (int i = 0; i < ACC_NUM; i++)
    {
        //acc[i].set_static_burst_debug(FLAG_SET);
        acc[i].init_memory(vertex_mem, dest_vertex_mem, weight);
        //acc[i].init_label_filter(metapath_lut, ARRAY_SIZE(metapath_lut));

    }
    double memcpy_end = getCurrentTimestamp();

    for (int i = 0; i < ACC_NUM; i++)
    {
        acc[i].init_res_path_memory(query_num / ACC_NUM * TEST_QD);
    }

    query_mem_t step_mem[ACC_NUM];
    std::vector<double> exec_time;

    double acc_stop;

    for (int i = 0; i < 10; i ++)
    {
        DEBUG_PRINTF("start test %d", i);
        for (size_t j = 0; j < ACC_NUM; j ++)
        {
            step_mem[j].clear();
            for (size_t k = 0; k < query_num / ACC_NUM; k++) {
                query_metadata_t q;
                //q.curr_vertex = pcg32_random() % vertex_num;
                q.curr_vertex = query_array[(k) % query_array.size()]; //pcg32_random() % vertex_num;
                //q.curr_vertex = 1;
                q.remain_step = TEST_QD;
                step_mem[j].push_back(q);
            }
        }

        //std::cout << "set_query\n";
        for (int j = 0; j < ACC_NUM ; j++)
        {
            acc[j].set_query(step_mem[j]);
        }
        //std::cout << "set_query done\n";
        double start = getCurrentTimestamp();
        for (int j = 0; j < ACC_NUM ; j++)
        {
            acc[j].start();
        }
        for (int j = 0; j < ACC_NUM ; j++)
        {
            acc[j].stop();
        }

        double stop = getCurrentTimestamp();
        exec_time.push_back(stop - start);
        acc_stop = getCurrentTimestamp();
        for (int j = 0; j < ACC_NUM ; j++)
        {
            acc[j].get_res();
        }
    }
    std::sort(exec_time.begin(), exec_time.end());
    double mid_exectime = exec_time[exec_time.size() / 2];
    DEBUG_PRINTF("[MID] exectime %lf", mid_exectime);
    double overall_thr =  vertex_num * TEST_QD / mid_exectime/1000/1000;

    DEBUG_PRINTF("[OVERALL THR] %lf", overall_thr);
    for (int j = 0; j < ACC_NUM ; j++)
    {
        acc[j].release();
    }
    double memout = getCurrentTimestamp();
    for (int nacc = 0; nacc < ACC_NUM ; nacc++)
    {
        path_mem_t res_record_mem = acc[nacc].get_paths();
        //std::vector<std::vector<uint32_t>> paths = get_path(res_record_mem, query_num / ACC_NUM * TEST_QD);

#if 0
// valid the sampled vertex is the neighour
        {
            uint32_t error = 0;
            uint32_t check_size = paths.size();
            for (uint32_t j = 0; j < check_size; j ++)
            {
                std::vector<uint32_t> path = paths[j];
                printf("path %d: %d", j, path[0]);
                for (uint32_t k = 1; k < path.size(); k++)
                {
                    uint32_t v = path[k];
#if 1
                    uint32_t curr_vertex = path[k - 1];
                    uint32_t flag_unmatched = FLAG_SET;

                    for (int deg = 0; deg < graph.degree(curr_vertex); deg++)
                    {
                        if (graph.adj_[graph.offset_[curr_vertex] + deg] == v)
                            flag_unmatched = FLAG_RESET;
                    }
                    if (flag_unmatched)
                    {
                        printf("neighbour test mismatch %d %d [%d]\n", curr_vertex, v, k);
                        error++;
                    }
#endif
                    printf(" %d", v);
                }
                //printf("\n");
            }
            printf("error :%d\n", error);
        }
#endif

#if 0
// valid the distribution
        {
#define MAX_DEGREE (8192)
#define BASE_V (0)
            uint32_t hist[MAX_DEGREE];
            for (int i = 0; i < MAX_DEGREE; i ++)
                hist[i] = 0;
            for (uint32_t j = 0; j < paths.size(); j ++)
            {
                std::vector<uint32_t> path = paths[j];
                for (uint32_t k = 1; k < path.size(); k++)
                {
                    uint32_t curr_vertex = path[k - 1];
                    uint32_t v = path[k];
                    if (k == (BASE_V + 1))
                    {
                        for (int deg = 0; deg < graph.degree(curr_vertex); deg++)
                        {
                            if (graph.adj_[graph.offset_[curr_vertex] + deg] == v)
                                hist[deg] ++;
                        }
                        break;
                    }

                }
            }
            uint32_t curr_vertex = paths[0][BASE_V];
            for (int deg = 0; deg < graph.degree(curr_vertex); deg++)
            {
                printf("%d --> %d \n", deg, hist[deg]);
            }
        }
#endif
    }
    double final_end = getCurrentTimestamp();

    DEBUG_PRINTF("[END] fpga init:  %lf", memcpy_start - fpga_start);
    DEBUG_PRINTF("[END] pcie memory copy: %lf", memcpy_end - memcpy_start);
    DEBUG_PRINTF("[END] acc execution:  %lf ", mid_exectime);
    DEBUG_PRINTF("[END] pcie result copy back %lf", memout - acc_stop);
    DEBUG_PRINTF("[END] overall thr (M Step/s): %lf", overall_thr);


    return 0;
}
