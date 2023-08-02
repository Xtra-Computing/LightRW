#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <xcl2.hpp>

#include "unistd.h"
#include "rw_data_struct.h"


#include "helper.h"
#include "pcg_basic.h"

#define TEST_SIZE (1)





const char * free_run_kernel_list[] = {

};

cl::Kernel      krnl_free_run[ARRAY_SIZE(free_run_kernel_list)];

#if 1
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];

    pcg32_srandom(42u, 54u);
    printf("starting \n");
    fflush(stdout);

    const int num_of_data = 1024;
    const int num_of_read = 256;

    std::vector<uint32_t, aligned_allocator<uint32_t> > buffer_1(num_of_data);
    std::vector<uint32_t, aligned_allocator<uint32_t> > buffer_2(num_of_data);
    std::vector<uint32_t, aligned_allocator<uint32_t> > res(num_of_read);
    for (size_t i = 0; i < num_of_data; i++) {
        buffer_1[i] = 1;
        buffer_2[i] = 2;
    }
    for (size_t i = 0; i < num_of_read; i++) {
        res[i] = 3;
    }


    cl_int err;
    cl::CommandQueue q;
    cl::CommandQueue free_run_q;
    cl::Context context;

    cl::Kernel      krnl_offset_loader;
    cl::Kernel      krnl_store;

    std::vector<cl::Device> devices = xcl::get_xil_devices();
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    cl::Device device;
    for (unsigned int i = 0; i < devices.size(); i++) {
        device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE //| CL_QUEUE_PROFILING_ENABLE
                                            , &err));
        OCL_CHECK(err, free_run_q = cl::CommandQueue(context, device,
                                    CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE //| CL_QUEUE_PROFILING_ENABLE
                                    , &err));

        std::cout << "Trying to program device[" << i << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";

            for (unsigned int j = 0; j < ARRAY_SIZE(free_run_kernel_list); j++)
            {
                OCL_CHECK(err, krnl_free_run[j] = cl::Kernel(program, free_run_kernel_list[j], &err));
            }

            OCL_CHECK(err, krnl_offset_loader = cl::Kernel(program, "offset_loader:{offset_loader_1}", &err));
            OCL_CHECK(err, krnl_store = cl::Kernel(program, "test_store_32:{test_store_32_1}", &err));

            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }

    OCL_CHECK(err, cl::Buffer buffer_in_1(context,
                                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          num_of_data * sizeof(uint32_t), buffer_1.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_in_2(context,
                                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          num_of_data * sizeof(uint32_t), buffer_2.data(), &err));
    OCL_CHECK(err, cl::Buffer  buffer_res(context,
                                          CL_MEM_USE_HOST_PTR,
                                          num_of_read * sizeof(uint32_t), res.data(), &err));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({   buffer_in_1,
                                                    }, 0 /*0 means from host*/));

    OCL_CHECK(err, err = q.finish());

    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({   buffer_in_2, buffer_res
                                                    }, 0 /*0 means from host*/));

    OCL_CHECK(err, err = q.finish());

    uint64_t address_buffer_1 = 0;
    uint64_t address_buffer_2 = 0;
    xclGetMemObjDeviceAddress(buffer_in_1(), device.get(), sizeof(uintptr_t), &address_buffer_1);
    xclGetMemObjDeviceAddress(buffer_in_2(), device.get(), sizeof(uintptr_t), &address_buffer_2);
    printf("addr_1 %16lx \n",  address_buffer_1);
    printf("addr_2 %16lx \n",  address_buffer_2);
    printf("addr offset %16lx \n", address_buffer_2 - address_buffer_1);


    uint32_t number_of_read = num_of_read;
    uint64_t offset = (address_buffer_2 - address_buffer_1)/sizeof(uint32_t);


    OCL_CHECK(err, err = krnl_offset_loader.setArg(0, number_of_read));
    OCL_CHECK(err, err = krnl_offset_loader.setArg(1, buffer_in_1));
    OCL_CHECK(err, err = krnl_offset_loader.setArg(3, offset));

    OCL_CHECK(err, err = krnl_store.setArg(0, number_of_read));
    OCL_CHECK(err, err = krnl_store.setArg(1, buffer_res));



    for (unsigned int j = 0; j < ARRAY_SIZE(free_run_kernel_list); j++)
    {
        OCL_CHECK(err, err = free_run_q.enqueueTask(krnl_free_run[j]));
    }


    std::vector<double> exec_time;
    //for (int i = 0; i < 5; i ++)
    {
        double start, stop;
        start = getCurrentTimestamp();
        OCL_CHECK(err, err = q.enqueueTask(krnl_store));
        //OCL_CHECK(err, err = q.enqueueTask(krnl_test_bs_loader));
        OCL_CHECK(err, err = q.enqueueTask(krnl_offset_loader));
        //OCL_CHECK(err, err = q.enqueueTask(krnl_sampler));

        stop = getCurrentTimestamp();
        printf("enqueue  time: %lf\n", stop - start);

        OCL_CHECK(err, err = q.finish());
        stop = getCurrentTimestamp();

        printf("exec time: %lf\n", stop - start);
        exec_time.push_back(stop - start);
        OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_res}, CL_MIGRATE_MEM_OBJECT_HOST));
        OCL_CHECK(err, err = q.finish());
    }
    for (int i = 0; i < 20; i ++)
    {
        printf("%d \n", res[i]);
    }
    //std::sort(exec_time.begin(), exec_time.end());
    //printf("[MID] exec time: %lf\n", exec_time[2]);

    return 0;
}

#else
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}
#endif