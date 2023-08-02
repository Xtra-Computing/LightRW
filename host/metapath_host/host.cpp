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

extern CL_API_ENTRY cl_int CL_API_CALL
xclGetMemObjDeviceAddress(cl_mem mem,
                          cl_device_id device,
                          size_t sz,
                          void* address);

//step_update fetch_next

#ifndef  ACC_NUM
#error "ACC_NUM is required"
#endif
const char * free_run_kernel_list[] = {

};

uint16_t metapath_lut[] = {1, 1, 5, 4, 3, 2, 1, 1}; // label can not be 0

const size_t max_weight_value = 5;




int main(int argc, char** argv) {

#include "accinit.h"
    //printf("test %d %d\n",RANGE_TEST(step_metadata_t, curr_vertex));
    //exit(0);



    double fpga_start = getCurrentTimestamp();
    accelerator acc[ACC_NUM];

    for (int i = 0; i < ACC_NUM; i++)
    {
        //acc[i].set_static_burst_debug(FLAG_SET);
        acc[i].init_acc(device, context, cprogram, i);
    }

    double memcpy_start = getCurrentTimestamp();
    for (int i = 0; i < ACC_NUM; i++)
    {
        acc[i].init_memory(vertex_mem, dest_vertex_mem, weight);
        acc[i].init_label_filter(metapath_lut, ARRAY_SIZE(metapath_lut));
    }
    double memcpy_end = getCurrentTimestamp();



    query_mem_t step_mem[ACC_NUM];
    std::vector<double> exec_time;

    for (int i = 0; i < 10; i ++)
    {
        DEBUG_PRINTF("start test %d", i);
        for (size_t j = 0; j < ACC_NUM; j ++)
        {
            step_mem[j].clear();
            for (size_t k = 0; k < query_num / ACC_NUM; k++) {
                query_metadata_t q;
                //q.curr_vertex = pcg32_random() % vertex_num;
                q.curr_vertex = query_array[k % query_array.size()]; //pcg32_random() % vertex_num;
                q.remain_step = 6;
                step_mem[j].push_back(q);
            }
        }

        for (int j = 0; j < ACC_NUM ; j++)
        {
            acc[j].set_query(step_mem[j]);
        }
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
        for (int j = 0; j < ACC_NUM ; j++)
        {
            acc[j].get_res();
        }
    }
    std::sort(exec_time.begin(), exec_time.end());
    double mid_exectime = exec_time[exec_time.size() / 2];
    DEBUG_PRINTF("[MID] exectime %lf", mid_exectime);
    double overall_thr = 0;
    for (size_t j = 0; j < ACC_NUM; j ++)
    {
        overall_thr  += acc[j].get_throughput();
        DEBUG_PRINTF("[MID THR] @%lu %lf", j, acc[j].get_throughput());
    }
    DEBUG_PRINTF("[OVERALL THR] %lf", overall_thr);
    for (int j = 0; j < ACC_NUM ; j++)
    {
        acc[j].release();
    }


    DEBUG_PRINTF("[END] fpga init:  %lf", memcpy_start - fpga_start);
    DEBUG_PRINTF("[END] pcie memory copy: %lf", memcpy_end - memcpy_start);
    DEBUG_PRINTF("[END] acc execution:  %lf ", mid_exectime);
    DEBUG_PRINTF("[END] overall thr (M Step/s): %lf", overall_thr);


    return 0;
}
