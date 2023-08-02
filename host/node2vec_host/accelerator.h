#ifndef __ACCELERATOR_H__
#define __ACCELERATOR_H__

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <vector>

#include "helper.h"


#include "rw_data_struct.h"
#include "metapath_data_struct.h"


typedef std::vector<query_metadata_t, aligned_allocator<query_metadata_t> > query_mem_t;

typedef std::vector<dummy_step_t, aligned_allocator<dummy_step_t> > dummy_step_mem_t;

typedef std::vector<vertex_descriptor_t, aligned_allocator<vertex_descriptor_t> > vd_mem_t;
typedef std::vector<uint32_t, aligned_allocator<uint32_t> > weight_mem_t;
typedef std::vector<uint32_t, aligned_allocator<uint32_t> > dest_mem_t;

typedef std::vector<metapath_sm_lut_t, aligned_allocator<metapath_sm_lut_t> > label_lut_mem_t;

typedef std::vector<uint64_t, aligned_allocator<uint64_t> > cycle_mem_t;

// TODO res memory
typedef std::vector<res_data_t, aligned_allocator<res_data_t> > res_mem_t;

// TODO path memory
typedef res_data_t path_data_t;
typedef std::vector<path_data_t, aligned_allocator<path_data_t> > path_mem_t;



#define MAX_KERNEL_STRING_LENGTH (256)


#define ADD_KERNEL(clkernel, name)  {                                                                       \
        char kernel_string[MAX_KERNEL_STRING_LENGTH];                                                       \
        int ns =  snprintf(kernel_string, ARRAY_SIZE(kernel_string),name, this->cu_index);                  \
        if (ns > MAX_KERNEL_STRING_LENGTH){ printf("Overflow error in add kernel: %s", name); exit(-1); }   \
        OCL_CHECK(err, clkernel = cl::Kernel(this->program, kernel_string, &err));                          \
}

#define ADD_KERNEL_INCR(clkernel, name, inc, offs)  {                                                       \
        char kernel_string[MAX_KERNEL_STRING_LENGTH];                                                       \
        int ns =  snprintf(kernel_string, ARRAY_SIZE(kernel_string),name, ((this->cu_index - 1) * inc) + offs + 1);     \
        if (ns > MAX_KERNEL_STRING_LENGTH){ printf("Overflow error in add kernel: %s", name); exit(-1); }   \
        OCL_CHECK(err, clkernel = cl::Kernel(this->program, kernel_string, &err));                          \
}

#define CU_ID_MAP(id)   (id + 1)

class accelerator
{
private:
    cl::Device device;
    cl::Context context;
    cl::Program program;

    cl::CommandQueue q;
    cl::CommandQueue q_array[10];

    cl::CommandQueue aux_q;
    cl::CommandQueue end_q;

    cl::Kernel      krnl_step_loader;
    cl::Kernel      krnl_vertex_loader;
    cl::Kernel      krnl_fetch_next;
    cl::Kernel      krnl_ring_manager;
    cl::Kernel      krnl_dynamic_burst;

    cl::Kernel      krnl_label_filter;

    cl::Kernel      krnl_sec_vertex_loader;
    cl::Kernel      krnl_sec_dynamic_burst;

    cl::Kernel      krnl_step_source;
    cl::Kernel      krnl_step_sink;
    cl::Buffer      step_in_buffer;
    cl::Buffer      step_out_buffer;


    cl::Kernel      krnl_sec_cmd;

    cl::Kernel      krnl_latency;

    cl::Buffer      query;
    cl::Buffer      vd;
    cl::Buffer      dest;
    cl::Buffer      res;
    cl::Buffer      lut;
    cl::Buffer      weight;
    cl::Buffer      siweight;
    cl::Buffer      sub_weight[5];


    cl::Buffer      sec_vd;

    res_mem_t       res_mem;
    label_lut_mem_t lut_mem;

    const int ring_num = 128;

    int record_enabled = 0;
    int record_memory_set = 0;

    cl::Buffer      path;
    cl::Kernel      krnl_step_record;
    path_mem_t      path_mem;

    cl::Buffer      latency_cycle;
    uint32_t        latency_counter;
    cycle_mem_t     cycle_mem;

    int static_burst_debug = 0;

public:
    int cu_index;
    cl_int err;
    std::vector<double> exec_time;
    std::vector<double> thr;

    double start_time, stop_time;
    double mem_start_time, mem_stop_time;

    accelerator()
    {
    }

    accelerator(cl::Device device, cl::Context context, cl::Program program, int index)
    {
        this->init_acc(device, context, program, index);
    }
    accelerator(cl::Device device, cl::Context context, cl::Program program, int index, int record_enabled)
    {
        this->record_enabled = record_enabled;
        this->init_acc(device, context, program, index);
    }

    int init_acc(cl::Device device, cl::Context context, cl::Program program, int index)
    {
        this->device = device;
        OCL_CHECK(err, this->context = cl::Context(this->device, nullptr, nullptr, nullptr, &err));
        this->program = program;
        this->cu_index = CU_ID_MAP(index);
        OCL_CHECK(err, this->q = cl::CommandQueue(this->context, this ->device,
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE //| CL_QUEUE_PROFILING_ENABLE
                                 , &err));
        #if 0
        for (int i = 0; i < 10; i++)
        {
             OCL_CHECK(err, this->q_array[i] = cl::CommandQueue(this->context, this ->device,
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE //| CL_QUEUE_PROFILING_ENABLE
                                 , &err));
        }
        #endif

        OCL_CHECK(err, this->aux_q = cl::CommandQueue(this->context, this ->device,
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE //| CL_QUEUE_PROFILING_ENABLE
                                 , &err));
        OCL_CHECK(err, this->end_q = cl::CommandQueue(this->context, this ->device,
                                 CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE //| CL_QUEUE_PROFILING_ENABLE
                                 , &err));

#if STEP_VERIFICATION
        ADD_KERNEL(this->krnl_step_source, "step_source:{step_source_%d}");
        ADD_KERNEL(this->krnl_step_sink, "step_sink:{step_sink_%d}");
#else
        ADD_KERNEL(this->krnl_step_loader, "step_loader:{step_loader_%d}");
        ADD_KERNEL(this->krnl_ring_manager, "ring_manager:{ring_manager_%d}");
#endif
        ADD_KERNEL(this->krnl_vertex_loader, "vertex_loader:{vertex_loader_%d}");
        //ADD_KERNEL(this->krnl_fetch_next, "fetch_next:{fetch_next_%d}");

        ADD_KERNEL(this->krnl_dynamic_burst, "staburst_core:{staburst_core_%d}");
        //ADD_KERNEL(this->krnl_vd_scheduler, "sta_vd_scheduler:{sta_vd_scheduler_%d}");

        //ADD_KERNEL_INCR(this->krnl_sec_vertex_loader, "vertex_loader:{vertex_loader_%d}", 2, 1);
        ADD_KERNEL(this->krnl_sec_dynamic_burst, "siburst_core:{siburst_core_%d}");
        //ADD_KERNEL(this->krnl_sec_vd_scheduler, "si_vd_scheduler:{si_vd_scheduler_%d}");
        //ADD_KERNEL(this->krnl_label_filter, "metapath_sm:{metapath_sm_%d}");

        //ADD_KERNEL(this->krnl_sec_cmd, "second_command:{second_command_%d}");
        if (this->record_enabled != 0)
        {
            ADD_KERNEL(this->krnl_step_record, "step_record:{step_record_%d}");
        }
#if LATENCY_TEST
        ADD_KERNEL(this->krnl_latency, "cycle_record:{cycle_record_%d}");
#endif

        return 0;
    }

    int init_acc(cl::Device device, cl::Context context, cl::Program program, int index, int record_enabled)
    {
        this->record_enabled = record_enabled;
        this->init_acc(device, context, program, index);
        return 0;
    }
    int init_memory(vd_mem_t &vd, dest_mem_t &dest, weight_mem_t &weight);
    int init_memory(vd_mem_t &vd, dest_mem_t &dest, weight_mem_t &weight, res_mem_t &res);


    int init_label_filter(uint16_t * lable, size_t  n);

    int set_query(query_mem_t &query);
    int set_query(dummy_step_mem_t &step_in, dummy_step_mem_t &step_out);

    int start_process_kernel(void);

    int init_res_path_memory(size_t num_of_node);

    path_mem_t get_paths(void);
    double get_res(void);
    double get_throughput(void);

    void set_static_burst_debug(int value)
    {
        static_burst_debug = value;
    }

    int exec(void);
    int start(void);
    int stop(void);
    int release(void);

    ~accelerator()
    {

    }

};



std::vector<std::vector<uint32_t>> get_path(path_mem_t &res, int total_size);


#endif /* __ACCELERATOR_H__*/

