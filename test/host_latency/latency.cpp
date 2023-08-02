/**
* Copyright (C) 2020 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/
#include "xcl2.hpp"
#include <vector>

#define DATA_SIZE  (1024)
#define INCR_VALUE 10
// define internal buffer max size
#define BURSTBUFFERSIZE 256



#include <stdint.h>
#include <stdio.h>

#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include "pcg_basic.h"



const size_t memsize = 128 * 1024 * 1024;
const size_t elems = memsize / sizeof(uint32_t);




void wait_for_enter(const std::string &msg) {
    std::cout << msg << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}


double getCurrentTimestamp(void) {
    timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    return (double(a.tv_nsec) * 1.0e-9) + double(a.tv_sec);
}


typedef struct
{
    uint32_t id;
    pthread_t tid;
    sem_t sem;
    uint32_t data;
    pcg32_random_t rng;
    int mode;
} thread_item_t;


#define MAX_NUM_OF_THREAD (4096)
thread_item_t local_thread[MAX_NUM_OF_THREAD];

#define prefetch (1)

void* test_thread(void* arg)
{
    thread_item_t* item = (thread_item_t *)(arg);
    unsigned int id = item->id;
    item->data = 0;
    pcg32_srandom_r(&item->rng, 42u, 54u + id );

    std::vector<int, aligned_allocator<int>> test_buffer(DATA_SIZE);
    //for (int i = 0; i < DATA_SIZE; i++) {
    //    test_buffer[i] = pcg32_random_r(&item->rng);
    //}

    uint32_t* data = (uint32_t* )malloc(memsize);

    for (uint32_t i = 0; i < elems; i++) {
        data[i] = i; //  page faults remove
    }
    sem_wait(&item->sem);
#if 0

    //printf("thread %d init done\n", id);

    if (item->mode == 0)
    {
        for (int i = 0; i < DATA_SIZE; i++) {
            unsigned int addr = pcg32_random_r(&item->rng) % DATA_SIZE;
            item->data += addr;
        }

    }
    else
    {
        for (int i = 0; i < DATA_SIZE; i++) {
            unsigned int addr = pcg32_random_r(&item->rng) % DATA_SIZE;

            item->data += test_buffer[addr];
        }
    }
#else
    uint32_t sum = 0;

    uint32_t buffer[prefetch];
    for (size_t i = 0; i < prefetch; i++)
        buffer[i] = pcg32_random() % elems;


    // Iteration countt to number of elems without resorting to modulo
    // prefetch or checking for two conditions inside inner
    // loops. Note that elems is a multiple of prefetch :)
    for (size_t i = 0; i < elems;) {
        for (size_t j = 0; j < prefetch; j++) {
            sum += data[buffer[j]];

            uint32_t next = pcg32_random() % elems;
            __builtin_prefetch(data + next);
            buffer[j] = next;
            i++;
        }
    }

#endif
    pthread_exit(NULL);

}



#define MAX_RECORDS_BITS (12)
#define MAX_RECORDS (1<< MAX_RECORDS_BITS)

std::vector<uint64_t, aligned_allocator<uint64_t>> source_cycles(MAX_RECORDS);

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string binaryFile = argv[1];

    pcg32_srandom(42u, 54u);

    const int size = DATA_SIZE;
    int inc_value = INCR_VALUE;
    cl_int err;
    cl::Kernel krnl_time_tag;
    cl::Kernel krnl_mem_read;
    cl::Kernel krnl_sink;
    cl::CommandQueue q;
    cl::Context context;
    // Allocate Memory in Host Memory
    size_t vector_size_bytes = sizeof(int) * size;
    std::vector<int, aligned_allocator<int>> source_inout(size);
    std::vector<int, aligned_allocator<int>> source_res(size);

    std::vector<int, aligned_allocator<int>> source_sw_results(size);

    // Create the test data and Software Result

    for (int i = 0; i < size; i++) {
        source_inout[i] = i;
        source_sw_results[i] = i + inc_value;
        source_res[i] = 0;
    }
    for (int i = 0; i< MAX_RECORDS; i++)
    {
        source_cycles[i] = 0;
    }
#if 0
    for (int num_of_test_threads = 1; num_of_test_threads < MAX_NUM_OF_THREAD; num_of_test_threads *= 2)
    {
        double startStamp, endStamp;
        printf("start with %d\n", num_of_test_threads);
        fflush(stdout);



        for (int i = 0; i < num_of_test_threads; i++)
        {
            local_thread[i].id = i;
            local_thread[i].mode = 1;
            sem_init(&local_thread[i].sem, 0, 0);
            pthread_create(&local_thread[i].tid, NULL, test_thread, (void *)&local_thread[i]) ;
        }

        sleep(5);

        startStamp = getCurrentTimestamp();
        for (int i = 0; i < num_of_test_threads; i ++)
        {
            sem_post(&local_thread[i].sem);
        }
        for (int i = 0; i < num_of_test_threads; i ++)
        {
            pthread_join(local_thread[i].tid, NULL);
        }
        endStamp = getCurrentTimestamp();
        std::cout << "raw time " <<  (endStamp - startStamp ) * 1000 << " ms"  << std::endl;
        std::cout << "time " <<  (endStamp - startStamp ) * 1000 - 61 << " ms"  << std::endl;
        std::cout << "num_of_test_threads " << num_of_test_threads << " bw " <<  (memsize * double(num_of_test_threads)) / (endStamp - startStamp) / 1000 / 1000 << std::endl;

    }

#endif

    // OPENCL HOST CODE AREA START
    auto devices = xcl::get_xil_devices();
    // read_binary_file() is a utility API which will load the binaryFile
    // and will return the pointer to file buffer.
    auto fileBuf = xcl::read_binary_file(binaryFile);
    cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
    bool valid_device = false;
    for (unsigned int i = 0; i < devices.size(); i++) {
        auto device = devices[i];
        // Creating Context and Command Queue for selected Device
        OCL_CHECK(err, context = cl::Context(device, NULL, NULL, NULL, &err));
        OCL_CHECK(err, q = cl::CommandQueue(context, device,
                                            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err));

        std::cout << "Trying to program device[" << i
                  << "]: " << device.getInfo<CL_DEVICE_NAME>() << std::endl;
        cl::Program program(context, {device}, bins, NULL, &err);
        if (err != CL_SUCCESS) {
            std::cout << "Failed to program device[" << i << "] with xclbin file!\n";
        } else {
            std::cout << "Device[" << i << "]: program successful!\n";
            OCL_CHECK(err, krnl_mem_read = cl::Kernel(program, "latency_test:{latency_test_1}", &err));
            OCL_CHECK(err, krnl_sink = cl::Kernel(program, "sink:{sink_1}", &err));
            OCL_CHECK(err, krnl_time_tag = cl::Kernel(program, "cycle_record:{cycle_record_1}", &err));

            valid_device = true;
            break; // we break because we found a valid device
        }
    }
    if (!valid_device) {
        std::cout << "Failed to program any device found, exit!\n";
        exit(EXIT_FAILURE);
    }
    //wait_for_enter("\nPress ENTER to continue after setting up ILA trigger...");

    // Allocate Buffer in Global Memory
    OCL_CHECK(err, cl::Buffer buffer_rw(
                  context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                  vector_size_bytes, source_inout.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_res(
                  context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                  vector_size_bytes, source_res.data(), &err));

    OCL_CHECK(err, cl::Buffer buffer_cycle(
                  context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                  source_cycles.size()* sizeof(uint64_t), source_cycles.data(), &err));



    OCL_CHECK(err, err = krnl_mem_read.setArg(0, buffer_rw));
    OCL_CHECK(err, err = krnl_sink.setArg(0, buffer_res));
    OCL_CHECK(err, err = krnl_time_tag.setArg(0, buffer_cycle));
    OCL_CHECK(err, err = krnl_mem_read.setArg(1, size));
    OCL_CHECK(err, err = krnl_sink.setArg(1, size));
    OCL_CHECK(err, err = krnl_time_tag.setArg(1, size));

    // Copy input data to device global memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_rw, buffer_res, buffer_cycle},
                         0 /* 0 means from host*/));
    OCL_CHECK(err, err = q.finish());

    // Launch the Kernel
    for (int i = 0; i < 4 ; i++)
    {
        double startStamp, endStamp;
        startStamp = getCurrentTimestamp();
        OCL_CHECK(err, err = q.enqueueTask(krnl_time_tag));
        OCL_CHECK(err, err = q.enqueueTask(krnl_sink));


        OCL_CHECK(err, err = q.enqueueTask(krnl_mem_read));

        OCL_CHECK(err, err = q.finish());
        endStamp = getCurrentTimestamp();
        std::cout << "time " <<  (endStamp - startStamp) * 1000 << std::endl;
        std::cout << "bw " <<  (size * 4 * 16) / (endStamp - startStamp) / 1000 / 1000 << std::endl;
        std::cout << "latency " <<   (endStamp - startStamp) / (size )  * 1000  * 1000  * 1000 << "ns" << std::endl;
    }
    // Copy Result from Device Global Memory to Host Local Memory
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_cycle},
                         CL_MIGRATE_MEM_OBJECT_HOST));
    OCL_CHECK(err, err = q.finish());
    uint64_t total = 0;

    #define PIPELINE_DEPTH (13 * 2)
    for (int i = 0; i < MAX_RECORDS; i++)
    {
        total+= source_cycles[i];
    }
    printf("%lf\n", (total/PIPELINE_DEPTH/ double(MAX_RECORDS) - 1) * 1.0/300*1000) ;


    double max = 0;
    double min = 100000000;
    for (int i = 0; i < MAX_RECORDS; i++)
    {

        double cycle = (double(source_cycles[i]))/PIPELINE_DEPTH - 1;
        if (max < cycle)
        {
            max = cycle;
        }
        if (min > cycle)
        {
            min = cycle;
        }
    }
    printf("%lf\n", min* 1.0/300*1000);
    printf("%lf\n", max* 1.0/300*1000);

    // OPENCL HOST CODE AREA END

    // Compare the results of the Device to the simulation
    return 0;
}
