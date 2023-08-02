#include <string>
#include <iostream>
#include <map>

#include "host_graph_sw.h"
#include "host_graph_api.h"


// XRT includes
#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#define USE_APPLY true
#define ROOT_NODE true // assume only root node have apply kernel


int acceleratorInit(int world_rank, int world_size, std::string& file_name,  graphInfo *info, graphAccelerator* acc)
{

    // load xclbin file
    acc->graphDevice = xrt::device(0); // every VM has only one FPGA, id = 0;
    acc->graphUuid = acc->graphDevice.load_xclbin(file_name);

    // init FPGA kernels

    for (int i = 0; i < (SUB_PARTITION_NUM / PROCESSOR_NUM); i++)
    {
        {
            std::string id, krnl_name;
            id = std::to_string(i + 1);
            krnl_name = "gs:{gs_" + id + "}";
            acc->gsKernel[i] = xrt::kernel(acc->graphDevice, acc->graphUuid, krnl_name.c_str());
        }
#if TEST_GP
        {
            std::string id, krnl_name;
            id = std::to_string(i + 1);
            krnl_name = "src_cache_read_engine:{src_cache_read_engine_" + id + "}";
            acc->srcReKernel[i] = xrt::kernel(acc->graphDevice, acc->graphUuid, krnl_name.c_str());
        }
#endif
    }


    // init FPGA buffers
    long unsigned int prop_size_bytes = 0;
    long unsigned int edge_size_bytes = 0;
    long unsigned int outDeg_size_bytes = 0;
    long unsigned int temp_prop_size = 0;

    acc->edgeBuffer.resize(info->partitionNum);
    acc->tempBuffer.resize(info->partitionNum);

    for (int i = 0; i < info->partitionNum; i++) {
        acc->edgeBuffer[i].resize((SUB_PARTITION_NUM / PROCESSOR_NUM));
        acc->tempBuffer[i].resize((SUB_PARTITION_NUM / PROCESSOR_NUM));

        for (int j = 0; j < (SUB_PARTITION_NUM / PROCESSOR_NUM); j++) {
            int proc_j = j * PROCESSOR_NUM + world_rank;
            edge_size_bytes = info->chunkProp[i][proc_j].edgeNumChunk * 2 * sizeof(int); // each edge has two vertex, vertex index uses int type;
            acc->edgeBuffer[i][j] = xrt::bo(acc->graphDevice, edge_size_bytes, acc->gsKernel[j].group_id(0));
            info->chunkEdgeData[i][proc_j] = acc->edgeBuffer[i][j].map<int*>();
#if TEST_GP
            prop_size_bytes = info->alignedCompressedVertexNum * sizeof(prop_t); // vertex prop;
            acc->propBuffer[j] = xrt::bo(acc->graphDevice, prop_size_bytes, acc->srcReKernel[j].group_id(0));
            info->chunkPropData[proc_j] = acc->propBuffer[j].map<int*>();
#endif
            temp_prop_size = info->chunkProp[i][proc_j].destVertexNumChunk * sizeof(prop_t); // temp vertex size
            acc->tempBuffer[i][j] = xrt::bo(acc->graphDevice, temp_prop_size, acc->gsKernel[j].group_id(1));
            info->chunkTempData[i][proc_j] = acc->tempBuffer[i][j].map<int*>();
        }
    }

    std::cout << "[INFO] Accelerator init done ! " << std::endl;
    return 0;
}

int accGatherScatterExecute (int super_step, int world_rank, int partition, graphInfo *info, graphAccelerator * acc) {

    std::cout << "GS " << super_step << " Execute at processor " << world_rank << " partition " << partition << " begin ...";

    fflush(stdout);
    if (partition < 0) return -1; // check the right parition id;

    int p = partition;
    for (int sp = 0; sp < (SUB_PARTITION_NUM / PROCESSOR_NUM); sp++) {
        int isp = world_rank + sp * PROCESSOR_NUM;
        acc->gsRun[sp].set_arg(0, acc->edgeBuffer[p][sp]);
        acc->gsRun[sp].set_arg(1, acc->tempBuffer[p][sp]);
        acc->gsRun[sp].set_arg(2, info->chunkProp[p][isp].edgeNumChunk * 2);
        acc->gsRun[sp].start();
#if TEST_GP
        acc->srcReRun[sp].set_arg(0, acc->propBuffer[sp]);
        acc->srcReRun[sp].start();
#endif
    }

    for (int sp = 0; sp < (SUB_PARTITION_NUM / PROCESSOR_NUM); sp++) {
        acc->gsRun[sp].wait();
#if TEST_GP
        acc->srcReRun[sp].wait();
#endif
    }

    std::cout << " ... end" << std::endl;
    return partition;
}



void partitionTransfer(int world_rank, graphInfo *info, graphAccelerator * acc)
{
    // Synchronize buffer content with device side


    for (int i = 0; i < info->partitionNum; i++) {
        for (int j = 0; j < (SUB_PARTITION_NUM / PROCESSOR_NUM); j++) {
#if TEST_GP
            acc->propBuffer[j].sync(XCL_BO_SYNC_BO_TO_DEVICE);
#endif
            acc->edgeBuffer[i][j].sync(XCL_BO_SYNC_BO_TO_DEVICE);
        }
    }

    std::cout << "[INFO] Sync data (prop, temp, edge, outDeg) to device " << std::endl;
}

void setAccKernelArgs(int world_rank, int world_size, graphInfo *info, graphAccelerator * acc)
{
    for (int p = 0; p < info->partitionNum; p++) {
        for (int sp = 0; sp < (SUB_PARTITION_NUM / PROCESSOR_NUM); sp++) {
            acc->gsRun[sp] = xrt::run(acc->gsKernel[sp]);
            acc->gsRun[sp].set_arg(4, 0);
            acc->gsRun[sp].set_arg(5, info->alignedCompressedVertexNum);
#if TEST_GP
            acc->srcReRun[sp] = xrt::run(acc->srcReKernel[sp]);
#endif
        }
    }


    std::cout << "[INFO] Set Initial Kernel args done" << std::endl;

}

void resultTransfer(graphInfo *info, graphAccelerator * acc, int run_counter)
{
    // Transfer device buffer content to host side
    acc->outRegBuffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    for (int sp = 0; sp < (SUB_PARTITION_NUM / PROCESSOR_NUM); sp++) {
#if TEST_GP
        if (run_counter % 2 == 0) {
            acc->propBufferNew[sp].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        } else {
            acc->propBuffer[sp].sync(XCL_BO_SYNC_BO_FROM_DEVICE);
        }
#endif

        // for (int size = 0; size < info->chunkProp[p][sp].destVertexNumChunk; size++) {
        //     std::cout << "[" << p <<"]["<< sp << "]["<< size << "]:" << info->chunkTempData[p][sp][size];
        //     if ((size + 1) % 5 == 0) std::cout << std::endl;
        // }

        // std::cout << std::endl;
    }
    std::cout << "[INFO] Sync data (temp, outReg) to host " << std::endl;

}

