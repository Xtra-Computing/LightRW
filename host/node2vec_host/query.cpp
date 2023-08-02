#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <xcl2.hpp>
#include "CL/cl_ext_xilinx.h"
#include "graph.h"
#include "rw_data_struct.h"

#include "pcg_basic.h"

#include "accelerator.h"
//{
//    for (int j = 0; j < 512; j ++)
//    {
//        step_metadata_t q = res_record_mem[j];
//        //uint32_t curr_vertex;
//        //uint32_t remain_step;
//        //uint16_t state;
//        //uint16_t ring_state;
//        //uint32_t inner_state;
//        printf("%d %d @%d %d %d %d\n", q.curr_vertex, q.remain_step, q.state, q.ring_state & 0x1FF, (q.ring_state >> 9) & 0x1, q.inner_state);
//    }
//
//}


std::vector<std::vector<uint32_t>> get_path(path_mem_t &res, int total_size)
{
    std::vector<std::vector<uint32_t>> paths;
    for (int i = 0; i < total_size; i++)
    {
        path_data_t q = res[i];
        uint32_t identified_path_num = paths.size();
        uint32_t curr_path_id = q.inner_state;
        uint32_t curr_vertex_id = (q.curr_vertex & (~0x80000000));
        //uint32_t curr_vertex_id = q.curr_vertex;
        if (curr_path_id >= identified_path_num)
        {
            std::vector<uint32_t> new_path;
            new_path.push_back(curr_vertex_id);
            //new_path.push_back(q.remain_step);
            paths.push_back(new_path);
            if (curr_path_id != identified_path_num)
            {
                printf("error path id, expect %d get %d\n", identified_path_num, curr_path_id );
            }
        }
        else
        {
            paths[curr_path_id].push_back(curr_vertex_id);
            //paths[curr_path_id].push_back(q.remain_step);
            //printf("%d %d\n", curr_path_id, q.remain_step);
        }
    }
    return paths;
}