#ifndef __FPGA_KERNEL_H__
#define __FPGA_KERNEL_H__

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <iostream>
#include <vector>

#include "xcl2.hpp"
#include "helper.h"

class cl_accelerator
{
private:
    cl::Device device;
    cl::Context context;
    cl::Program program;
    cl_int err;
public:
    cl_accelerator(int argc, char *argv[])
    {
        std::string binaryFile;
        if (argc < 2) {
            std::string app_name = argv[0];
            app_name = app_name.substr(2, app_name.size() - 6);
            binaryFile = "build_dir_" + app_name + "/kernel.xclbin";
            printf("try to use default xclbin: %s\n", binaryFile.c_str());
        }
        else { binaryFile = argv[1]; }
        printf("starting \n");
        fflush(stdout);
        init_platform(binaryFile);


    }

    cl_accelerator(std::string binaryFile)
    {
        init_platform(binaryFile);
    }

    int init_platform(std::string binaryFile)
    {
        std::vector<cl::Device> devices = xcl::get_xil_devices();
        auto fileBuf = xcl::read_binary_file(binaryFile);
        cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
        bool valid_device = false;
        for (unsigned int i = 0; i < devices.size(); i++) {
            auto local_device = devices[i];
            OCL_CHECK(err, context = cl::Context(local_device, nullptr, nullptr, nullptr, &err));
            std::cout << "Trying to program device[" << i << "]: " << local_device.getInfo<CL_DEVICE_NAME>() << std::endl;
            cl::Program local_program(context, {local_device}, bins, nullptr, &err);
            if (err != CL_SUCCESS) {
                std::cout << "Failed to program device[" << i << "] with xclbin file\n";
            } else {
                std::cout << "Device[" << i << "]: program successful!\n";
                valid_device = true;
                program = local_program;
                device = local_device;
                break;
            }
        }
        if (!valid_device) {
            std::cout << "Failed to program any device found, exit!\n";
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    cl::CommandQueue alloc_task_queue(void)
    {
        cl::CommandQueue q;
        OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
                                            , &err));
        return q;
    }
    cl::Program get_program(void)
    {
        return program;
    }
    cl::Context get_context(void)
    {
        return context;
    }
    cl::Device get_device(void)
    {
        return device;
    }

};

#define MAX_KERNEL_STRING_LENGTH (1024)

class cl_krnl
{
protected:
    cl_int err;
    cl::CommandQueue mem_q;
    cl::Context context;
    double start, stop;
    bool init_flag = false;
    char kernel_string[MAX_KERNEL_STRING_LENGTH];

    void kernel_init(cl_accelerator &acc)
    {
        OCL_CHECK(err, krnl = cl::Kernel(acc.get_program(), kernel_string, &err));

        OCL_CHECK(err, mem_q = cl::CommandQueue(acc.get_context(), acc.get_device(),
                                                CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE
                                                , &err));
        context = acc.get_context();
    }

public:
    cl::Kernel krnl;

    cl_krnl(){}

    cl_krnl(cl_accelerator &acc, std::string name, uint32_t id)
    {
        int ns =  snprintf(kernel_string, ARRAY_SIZE(kernel_string), "%s:{%s_%d}",
                           name.c_str(),
                           name.c_str(),
                           id + 1);
        if (ns >= MAX_KERNEL_STRING_LENGTH)
        { printf("Overflow error in add kernel: %s", kernel_string); exit(-1); }
        kernel_init(acc);
    }

    //with category id
    cl_krnl(cl_accelerator &acc, std::string name, uint32_t cid, uint32_t id)
    {
        int ns =  snprintf(kernel_string, ARRAY_SIZE(kernel_string), "%s_%d:{%s_%d_%d}",
                           name.c_str(),
                           cid,
                           name.c_str(),
                           cid,
                           id + 1);
        if (ns >= MAX_KERNEL_STRING_LENGTH)
        { printf("Overflow error in add kernel: %s", kernel_string); exit(-1); }
        kernel_init(acc);
    }

    void set(int arg_id, cl::Buffer  &buffer)
    {
        OCL_CHECK(err, err = krnl.setArg(arg_id, buffer));
    }


    int schedule_task(cl::CommandQueue & q)
    {
        if (init_flag)
        {
            OCL_CHECK(err, err = q.enqueueTask(krnl));
        }
        else
        {
            printf("Uninitialization error in kernel: %s", kernel_string); exit(-1);
        }
        return 0;
    }

    int exec(void)
    {
        DEBUG_ERROR("can not exec alone\n")
        return 0;
    }

    int get_res(void)
    {
        DEBUG_ERROR("not implemented\n")
        return 0;
    }

    bool set_init_done(void)
    {
        //only inited kernel can be scheduled to task queue
        init_flag = true;
        return init_flag;
    }


};





#endif /* __FPGA_KERNEL_H__ */
