#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <xcl2.hpp>
#include <inttypes.h>

#include "CL/cl_ext_xilinx.h"
#include "graph.h"


#include "accelerator.h"



int accelerator::init_res_path_memory(size_t num_of_node)
{
    if (this->record_enabled)
    {
        path_mem.resize(num_of_node );
        for (size_t i = 0; i < num_of_node; i++) {
            path_data_t q;
            memset((void *)&q, 0 , sizeof (path_data_t));
            path_mem[i] = q;
        }

        OCL_CHECK(err, cl::Buffer res_path(this->context,
                                           CL_MEM_USE_HOST_PTR,
                                           path_mem.size() * sizeof(res_data_t), path_mem.data(), &err));
        this->path = res_path;
        OCL_CHECK(err, err = this->krnl_step_record.setArg(0, this->path));
        OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({
            this->path
        }, 0 /*0 means from host*/));
        TRACE();
        OCL_CHECK(err, err = this->q.finish());
        TRACE();
        record_memory_set = FLAG_SET;
    }
    return 0;
}

int accelerator::init_label_filter(uint16_t * lable, size_t  n)
{
    lut_mem.clear();
    for (size_t i = 0 ; i < n; i++)
    {
        metapath_sm_lut_t lut_item;
        lut_item.state = lable[i];
        lut_mem.push_back(lut_item);
    }
    lut_mem.resize(n);

    OCL_CHECK(err, cl::Buffer buffer_lut_mem(context,
              CL_MEM_USE_HOST_PTR,
              lut_mem.size() * sizeof(metapath_sm_lut_t), lut_mem.data(), &err));
    this->lut = buffer_lut_mem;
    OCL_CHECK(err, err = this->krnl_label_filter.setArg(0, this->lut));

    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({
        this->lut
    }, 0 /*0 means from host*/));
    OCL_CHECK(err, err = this->q.finish());
    return 0;

}

path_mem_t accelerator::get_paths(void)
{
    return path_mem;
}


int accelerator::init_memory(vd_mem_t &vd, dest_mem_t &dest, weight_mem_t &weight, res_mem_t &unused)
{
    return this->init_memory(vd, dest, weight);
}

int accelerator::init_memory(vd_mem_t &vd, dest_mem_t &dest, weight_mem_t &weight)
{
    mem_start_time = getCurrentTimestamp();
    res_mem.resize(1);
    res_data_t res_data;
    memset((void *)&res_data, 0 , sizeof (res_data_t));
    res_mem[0] = res_data;


    OCL_CHECK(err, cl::Buffer buffer_vertex_mem(this->context,
              CL_MEM_USE_HOST_PTR,
              vd.size() * sizeof(vertex_descriptor_t), vd.data(), &err));
    this->vd = buffer_vertex_mem;



    OCL_CHECK(err, cl::Buffer buffer_dest_vertex_mem(this->context,
              CL_MEM_USE_HOST_PTR,
              dest.size() * sizeof(uint32_t), dest.data(), &err));
    this->dest = buffer_dest_vertex_mem;


    OCL_CHECK(err, cl::Buffer buffer_weight_base(this->context,
              CL_MEM_USE_HOST_PTR,
              weight.size() * sizeof(uint32_t), weight.data(), &err));

    OCL_CHECK(err, cl::Buffer buffer_wdest_vertex_mem(this->context,
              CL_MEM_USE_HOST_PTR,
              dest.size() * sizeof(uint32_t), dest.data(), &err));

    this->weight = buffer_wdest_vertex_mem;

    OCL_CHECK(err, cl::Buffer buffer_res_mem(context,
              CL_MEM_USE_HOST_PTR,
              res_mem.size() * sizeof(res_data_t), res_mem.data(), &err));
    this->res = buffer_res_mem;

    //printf("[DUMP] memory size: %lf\n", (dest.size() * sizeof(uint32_t)
    //                                     + query.size() * sizeof(step_metadata_t) * 2
    //                                     + dest.size() * sizeof(uint32_t)
    //                                     + weight.size() * sizeof(uint32_t)) / 1000.0 / 1000);

    //printf("[DUMP] buffer_vertex_mem size: %lf\n", (vertex_mem_size) / 1000.0 / 1000);
    //printf("[DUMP] buffer_update_step_mem size: %lf\n", (query_mem_size) / 1000.0 / 1000);
    //printf("[DUMP] buffer_dest_vertex_mem size: %lf\n", (dest_vertex_mem_size) / 1000.0 / 1000);
    //printf("[DUMP] buffer_weight_base size: %lf\n", (dest_vertex_mem_size) / 1000.0 / 1000);
    //printf("[DUMP] buffer_step_mem size: %lf\n", (query_mem_size) / 1000.0 / 1000);

    //cl_buffer_region    subbufer_weight[5];
    //for (int i = 0; i < 5; i ++)
    //{
    //    subbufer_weight[i] = {0, weight.size() * sizeof(uint32_t)};
    //}

    cl_buffer_region    subbufer_sec_loader = {0, vd.size() * sizeof(vertex_descriptor_t)};;
    TRACE()

    cl_uint num_args;
    clGetKernelInfo(krnl_vertex_loader(), CL_KERNEL_NUM_ARGS, sizeof(cl_uint), &num_args, NULL);
    DEBUG_PRINTF("[DUMP] num_args  %d", num_args);
    for (int k = 0; k < num_args; k++)
    {
        char name_buff[100];
        cl_int ret = clGetKernelArgInfo(krnl_vertex_loader(), k, CL_KERNEL_ARG_TYPE_NAME, 100 * sizeof(char), &name_buff, NULL);
        DEBUG_PRINTF("[DUMP] name  %s %d", name_buff,ret);
    }

    //OCL_CHECK(err, err = this->krnl_fetch_next.setArg(0, this->res));
#if STEP_VERIFICATION
#else
    OCL_CHECK(err, err = this->krnl_ring_manager.setArg(3, this->ring_num));
#endif

    OCL_CHECK(err, err = this->krnl_dynamic_burst.setArg(0, this->weight));
    OCL_CHECK(err, err = this->krnl_vertex_loader.setArg(1, this->vd));
    TRACE()
    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({
        this->vd,
        //this->res,
        this->weight
    }, 0 /*0 means from host*/));
    OCL_CHECK(err, err = this->q.finish());
    TRACE()

    //this->sec_vd = this->vd.createSubBuffer(CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &subbufer_sec_loader);


    OCL_CHECK(err, cl::Buffer buffer_sec_vertex_mem(this->context,
              CL_MEM_USE_HOST_PTR,
              vd.size() * sizeof(vertex_descriptor_t), vd.data(), &err));
    this->sec_vd = buffer_sec_vertex_mem;
    //OCL_CHECK(err, err = this->krnl_sec_vertex_loader.setArg(1, this->sec_vd));


    if (static_burst_debug)
    {
        DEBUG_PRINTF("disabling sub-buffer for static evaluation");
    }
    else {

        //uint64_t address = 0;
        //xclGetMemObjDeviceAddress(buffer_dest_vertex_mem(), device.get(), sizeof(uintptr_t), &address);
        //printf("addr %16lx \n", address);

        // Create sub-buffers from buffers based on sub-buffer regions
        for (int i = 0; i < 5; i ++)
        {
            //this->sub_weight[i] = this->weight.createSubBuffer(CL_MEM_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION, &subbufer_weight[i]);
        }


    }
    TRACE()

    OCL_CHECK(err, cl::Buffer buffer_siwdest_vertex_mem(this->context,
              CL_MEM_USE_HOST_PTR,
              dest.size() * sizeof(uint32_t), dest.data(), &err));
    this->siweight = buffer_siwdest_vertex_mem;

    //OCL_CHECK(err, cl::Buffer buffer_w2_vertex_mem(this->context,
    //          CL_MEM_USE_HOST_PTR,
    //          vd.size() * sizeof(vertex_descriptor_t), vd.data(), &err));
    //this->sub_weight[2] = buffer_w2_vertex_mem;
    //OCL_CHECK(err, cl::Buffer buffer_w3_vertex_mem(this->context,
    //          CL_MEM_USE_HOST_PTR,
    //          vd.size() * sizeof(vertex_descriptor_t), vd.data(), &err));
    //this->sub_weight[3] = buffer_w3_vertex_mem;
    //OCL_CHECK(err, cl::Buffer buffer_w4_vertex_mem(this->context,
    //          CL_MEM_USE_HOST_PTR,
    //          vd.size() * sizeof(vertex_descriptor_t), vd.data(), &err));
    //this->sub_weight[4] = buffer_w4_vertex_mem;


    OCL_CHECK(err, err = this->krnl_sec_dynamic_burst.setArg(0, this->siweight));

    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({
        this->siweight,
    }, 0 /*0 means from host*/));
    OCL_CHECK(err, err = this->q.finish());

    mem_stop_time = getCurrentTimestamp();
    DEBUG_PRINTF("memory transfer time: %lf", mem_stop_time - mem_start_time);
    DEBUG_PRINTF("memory init done");
    return 0;
}
int accelerator::set_query(dummy_step_mem_t &step_in, dummy_step_mem_t &step_out)
{
    OCL_CHECK(err, cl::Buffer buffer_step_in_mem(context,
              CL_MEM_USE_HOST_PTR,
              step_in.size() * sizeof(dummy_step_t), step_in.data(), &err));
    this->step_in_buffer = buffer_step_in_mem;
    OCL_CHECK(err, cl::Buffer buffer_step_out_mem(context,
              CL_MEM_USE_HOST_PTR,
              step_out.size() * sizeof(dummy_step_t), step_out.data(), &err));
    this->step_out_buffer = buffer_step_out_mem;
    int step_num =  step_in.size();
    OCL_CHECK(err, err = this->krnl_step_source.setArg(0, step_num));
    OCL_CHECK(err, err = this->krnl_step_source.setArg(1, this->step_in_buffer));

    OCL_CHECK(err, err = this->krnl_step_sink.setArg(0, step_num));
    OCL_CHECK(err, err = this->krnl_step_sink.setArg(1, this->step_out_buffer));

    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({this->step_in_buffer, this->step_out_buffer }, 0 /*0 means from host*/));
    OCL_CHECK(err, err = this->q.finish());

    OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_step_sink));
    return this->start_process_kernel();
}

int accelerator::set_query(query_mem_t &query)
{
    OCL_CHECK(err, cl::Buffer buffer_step_mem(context,
              CL_MEM_USE_HOST_PTR,
              query.size() * sizeof(query_metadata_t), query.data(), &err));
    OCL_CHECK(err, err = this->krnl_step_loader.setArg(0, buffer_step_mem));
    int query_num =  query.size();

#if LATENCY_TEST
    this->latency_counter = query_num / 256;
    cycle_mem.clear();
    for (int i = 0; i <  this->latency_counter; i++)
    {
        uint32_t init_cycle = 0;
        cycle_mem.push_back(init_cycle);
    }
    OCL_CHECK(err, cl::Buffer buffer_cycle_mem(context,
              CL_MEM_USE_HOST_PTR,
              cycle_mem.size() * sizeof(uint32_t), cycle_mem.data(), &err));
    this->latency_cycle = buffer_cycle_mem;
    OCL_CHECK(err, err = this->krnl_latency.setArg(0, buffer_cycle_mem));
    OCL_CHECK(err, err = this->krnl_latency.setArg(1, this->latency_counter));
    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({buffer_cycle_mem}, 0 /*0 means from host*/));
    OCL_CHECK(err, err = this->q.finish());
#endif

    OCL_CHECK(err, err = this->krnl_step_loader.setArg(2,  query_num));
    OCL_CHECK(err, err = this->krnl_step_loader.setArg(3,  10));
    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({buffer_step_mem}, 0 /*0 means from host*/));
    OCL_CHECK(err, err = this->q.finish());
#if LATENCY_TEST
    OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_latency));
#endif
    return this->start_process_kernel();
}

int accelerator::start_process_kernel(void)
{

    //OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_sec_vertex_loader));

    //OCL_CHECK(err, err = this->end_q.enqueueTask(this->krnl_fetch_next));
#if STEP_VERIFICATION
#else
    OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_ring_manager));
#endif
    //OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_vd_scheduler));
    //OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_sec_vd_scheduler));
    OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_dynamic_burst));
    OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_sec_dynamic_burst));
    //OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_sec_cmd));
    if (this->record_enabled)
    {
        OCL_CHECK(err, err = this->aux_q.enqueueTask(this->krnl_step_record));
    }

    OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_vertex_loader));

    return 0;
}

int accelerator::exec(void)
{
    this->start();
    this->stop();
    return 0;
}

int accelerator::start(void)
{
    if (((this->record_enabled) && (this->record_memory_set)) || (this->record_enabled == 0))
    {
        start_time = getCurrentTimestamp();
#if STEP_VERIFICATION
        OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_step_source));
#else
        OCL_CHECK(err, err = this->q.enqueueTask(this->krnl_step_loader));
#endif
    }
    return 0;
}

int accelerator::stop(void)
{
    if (((this->record_enabled) && (this->record_memory_set)) || (this->record_enabled == 0))
    {
        OCL_CHECK(err, err = this->q.finish());
        stop_time = getCurrentTimestamp();
        DEBUG_PRINTF("[END] exec time: %lf", stop_time - start_time);
        this->exec_time.push_back(stop_time - start_time);
    }
    return 0;
}

int accelerator::release(void)
{
    //clReleaseCommandQueue(this->q);
    //clReleaseContext(this->context);
    return 0;

}

double accelerator::get_res(void)
{

    OCL_CHECK(err, err = this->aux_q.finish());

    //OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({this->res}, CL_MIGRATE_MEM_OBJECT_HOST));
    //OCL_CHECK(err, err = this->q.finish());
    if (this->record_enabled)
    {
        OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({this->path}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = this->q.finish());
    }
#if LATENCY_TEST
    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({this->latency_cycle}, CL_MIGRATE_MEM_OBJECT_HOST));
    OCL_CHECK(err, err = this->q.finish());
    for (int i = 0; i <  this->latency_counter; i++)
    {
        if (cycle_mem[i] != 0)
            DEBUG_PRINTF("%ld", cycle_mem[i]);
    }
#endif

#if STEP_VERIFICATION
    OCL_CHECK(err, err = this->q.enqueueMigrateMemObjects({this->step_out_buffer}, CL_MIGRATE_MEM_OBJECT_HOST));
    OCL_CHECK(err, err = this->q.finish());
#endif
    //std::sort(this->exec_time.begin(), this->exec_time.end());
    double exectime = this->exec_time[this->exec_time.size() - 1];
#if 1
    {
        res_data_t q = this->res_mem[0];
        this->thr.push_back(double(q.curr_vertex) / (exectime) / 1000 / 1000);
    }

#endif
    //printf("[MID] exec time: %lf\n", mid_exectime);
    return exectime;

}

double accelerator::get_throughput(void)
{
    if (this->thr.size() > 0)
    {
        std::sort(this->thr.begin(), this->thr.end());
        return this->thr[this->thr.size() / 2];
    }
    else
        return 0.0;
}
