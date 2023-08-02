
#include "host_graph_sw.h"
#include <map>
#include <algorithm>
// XRT includes
#include "experimental/xrt_bo.h"
#include "experimental/xrt_device.h"
#include "experimental/xrt_kernel.h"

#define EDEG_MEMORY_SIZE        ((edgeNum + (ALIGN_SIZE * 4) * 128) * 1)
#define VERTEX_MEMORY_SIZE      (((vertexNum - 1)/MAX_VERTICES_IN_ONE_PARTITION + 1) * MAX_VERTICES_IN_ONE_PARTITION)


int acceleratorDataLoad(const std::string &gName, const std::string &mode, graphInfo *info)
{

    int read_size = 0;
    std::string name;
    // read csr file : input row point array
    name = mode + gName + "_rapi.bin";
    std::ifstream fFileReader_rapi(name, std::ios::binary);
    if (!fFileReader_rapi) {
        std::cout << "graph rapi read file failed" << std::endl;
        return -1;
    }
    read_size = 0;
    fFileReader_rapi.read((char*)&read_size, sizeof(int)); // first number -> size;
    std::cout << " rapi file read file size : " << read_size << std::endl;
    int* graph_rapi = new int[read_size];
    fFileReader_rapi.read((char*)graph_rapi, read_size * sizeof(int));
    fFileReader_rapi.close();

    // read csr file : output row point array
    name = mode + gName + "_rapo.bin";
    std::ifstream fFileReader_rapo(name, std::ios::binary);
    if (!fFileReader_rapo) {
        std::cout << "graph rapo read file failed" << std::endl;
        return -1;
    }
    read_size = 0;
    fFileReader_rapo.read((char*)&read_size, sizeof(int)); // first number -> size;
    std::cout << " rapo file read file size : " << read_size << std::endl;
    int vertex_num = read_size;
    int* graph_rapo = new int[read_size];
    fFileReader_rapo.read((char*)graph_rapo, read_size * sizeof(int));
    fFileReader_rapo.close();

    // read ciai file : input column array
    name = mode + gName + "_ciai.bin";
    std::ifstream fFileReader_ciai(name, std::ios::binary);
    if (!fFileReader_ciai) {
        std::cout << "graph ciai read file failed" << std::endl;
        return -1;
    }
    read_size = 0;
    fFileReader_ciai.read((char*)&read_size, sizeof(int)); // first number -> size;
    std::cout << " ciai file read file size : " << read_size << std::endl;
    int edge_num = read_size;
    int* graph_ciai = new int[read_size];
    fFileReader_ciai.read((char*)graph_ciai, read_size * sizeof(int));
    fFileReader_ciai.close();

    info->vertexNum = vertex_num;
    info->edgeNum   = edge_num;

    // compress function : delete vertics whose OutDeg = 0;
    int num_mapped = 0;
    int num_deleted = 0;

    int* mapping_vertex_array = new int[vertex_num];
    for (int i = 0; i < vertex_num; i++) {
        mapping_vertex_array[i] = num_mapped;
        int outdeg_tmp = graph_rapo[i + 1] - graph_rapo[i];
        if (outdeg_tmp > 0) {
            info->outDeg.push_back(outdeg_tmp);
            info->vertexMapping.push_back(i);
            num_mapped++;
        } else {
            info->outDegZeroIndex.push_back(i);
            num_deleted++;
        }
    }

    if ((num_mapped + num_deleted) != vertex_num) {
        std::cout << "graph vertex compress error!" << std::endl; // double check
    }

    info->compressedVertexNum = num_mapped;

    int aligned_vertex_num = ((num_mapped + 1023) / 1024) * 1024;
    info->alignedCompressedVertexNum = aligned_vertex_num;

    std::cout << "Vertex compress: original : " << vertex_num << " compressed : " << num_mapped << " aligned: " << aligned_vertex_num << std::endl;

    // compress function : delete edges whose dest vertex's OutDeg = 0;
    int num_compress_edge = 0;
    info->rpa.resize(num_mapped + 1);
    info->rpa[0] = 0;
    for (int j = 0; j < num_mapped; j++) {
        int in_deg_tmp = graph_rapi[info->vertexMapping[j] + 1] - graph_rapi[info->vertexMapping[j]];
        info->rpa[j + 1] = info->rpa[j] + in_deg_tmp;
        int bias = graph_rapi[info->vertexMapping[j]];
        for (int k = 0; k < in_deg_tmp; k++) {
            info->cia.push_back(mapping_vertex_array[graph_ciai[bias + k]]);
            info->destIndexList.push_back(j); // for time optimization in partition function.
            num_compress_edge++;
        }
    }

    if ( num_compress_edge != info->rpa[num_mapped]) {
        std::cout << "graph edge compress error!" << std::endl; // double check
    }

    std::cout << "Edge compress: original : " << edge_num << " compressed : " << num_compress_edge << std::endl;
    info->compressedEdgeNum = num_compress_edge;

    delete []graph_rapo;
    delete []graph_rapi;
    delete []graph_ciai;

    // calculate partition number and edge number in each subpartition
    int num_partition = (num_mapped + PARTITION_SIZE - 1) / PARTITION_SIZE; // PARTITION_SIZE align
    int num_subpartition = SUB_PARTITION_NUM;
    info->partitionNum = num_partition;

    int vertex_index_start = 0;
    int vertex_index_end = 0;

    info->chunkProp.resize(num_partition);
    info->chunkEdgeData.resize(num_partition);

    info->chunkOutDegData = new int[info->alignedCompressedVertexNum];
    info->chunkOutRegData = new int[info->alignedCompressedVertexNum];
    std::fill_n(info->chunkOutDegData, info->alignedCompressedVertexNum, 0);
    std::fill_n(info->chunkOutRegData, info->alignedCompressedVertexNum, 0);

    info->chunkTempData.resize(num_partition);
    info->chunkPropData.resize(SUB_PARTITION_NUM);
    info->chunkPropDataNew.resize(SUB_PARTITION_NUM);

    for (int i = 0; i < num_partition; i++) {
        info->chunkProp[i].resize(num_subpartition);
        info->chunkEdgeData[i].resize(num_subpartition);
        info->chunkTempData[i].resize(num_subpartition);
    }

    for (int p = 0; p < num_partition; p++) {

        vertex_index_start = p * PARTITION_SIZE;
        vertex_index_end = ((p + 1) * PARTITION_SIZE > num_mapped) ? num_mapped : (p + 1) * PARTITION_SIZE;

        int edge_num_tmp = 0;
        edge_num_tmp = info->rpa[vertex_index_end] - info->rpa[vertex_index_start]; // for each partition
        edge_num_tmp = (edge_num_tmp + num_subpartition - 1) / num_subpartition; // for each subpartition;
        edge_num_tmp = ((edge_num_tmp + ALIGN_SIZE - 1) / ALIGN_SIZE) * ALIGN_SIZE; // for alignment

        for (int sp = 0; sp < num_subpartition; sp++) {
            info->chunkProp[p][sp].edgeNumChunk = edge_num_tmp;
            info->chunkProp[p][sp].destVertexNumChunk = (vertex_index_end - vertex_index_start + 1023) / 1024 * 1024; // aligned vertex number

            info->chunkEdgeData[p][sp] = new int[edge_num_tmp * 2]; // each edge has 2 vertics.
            info->chunkTempData[p][sp] = new int[vertex_index_end - vertex_index_start];
            info->chunkPropData[sp] = new int[info->alignedCompressedVertexNum];
            info->chunkPropDataNew[sp] = new int[info->alignedCompressedVertexNum];

            std::fill_n(info->chunkTempData[p][sp], vertex_index_end - vertex_index_start, 0);
            std::fill_n(info->chunkPropData[sp], info->alignedCompressedVertexNum, 0);
            std::fill_n(info->chunkPropDataNew[sp], info->alignedCompressedVertexNum, 0);
            std::fill_n(info->chunkEdgeData[p][sp], edge_num_tmp * 2, 0);
        }

        std::cout << "Partition " << p << " start index " << vertex_index_start << " end index " << vertex_index_end << std::endl;
        std::cout << "Edge number: partion " << edge_num_tmp * num_subpartition << " subpartion " << edge_num_tmp << std::endl;
    }

    delete [] mapping_vertex_array;
    return 0;
}


void reTransferProp(graphInfo *info)
{
//     dataPrepareProperty(info);
//     graphAccelerator * acc = getAccelerator();

//     int *rpa = (int*)get_host_mem_pointer(MEM_ID_RPA);
//     prop_t *vertexPushinPropMapped = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP_MAPPED);
//     prop_t *vertexPushinProp       = (prop_t*)get_host_mem_pointer(MEM_ID_PUSHIN_PROP);
//     unsigned int *vertexMap        = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_MAP);
//     unsigned int *vertexRemap      = (unsigned int *)get_host_mem_pointer(MEM_ID_VERTEX_INDEX_REMAP);
//     unsigned int mapedSourceIndex  = 0;
//     int vertexNum = info->vertexNum;

//     for (int u = 0; u < vertexNum; u++) {
//         int num = rpa[u + 1] - rpa[u];
//         vertexMap[u] = mapedSourceIndex;
//         if (num != 0)
//         {
// #if CAHCE_FETCH_DEBUG
//             vertexPushinPropMapped[mapedSourceIndex] =  mapedSourceIndex;
// #else
//             vertexPushinPropMapped[mapedSourceIndex] =  vertexPushinProp[u];
// #endif
//             vertexRemap[mapedSourceIndex] = u;
//             mapedSourceIndex ++;
//         }
//     }
//     process_mem_init(acc->context);
//     partitionTransfer(info);
}


void partitionFunction(graphInfo *info)
{
    std::cout << "[INFO] Start partition function " << std::endl;
    int vertex_index_start = 0;
    int vertex_index_end = 0;

    std::vector<std::multimap<int, int>> edge_list;
    edge_list.resize(info->partitionNum);

    for (int p = 0; p < info->partitionNum; p++) {

        vertex_index_start = p * PARTITION_SIZE;
        vertex_index_end = ((p + 1) * PARTITION_SIZE > (info->compressedVertexNum)) ? (info->compressedVertexNum) : (p + 1) * PARTITION_SIZE;

        for (int i = info->rpa[vertex_index_start]; i < info->rpa[vertex_index_end]; i++) {
            edge_list[p].insert(std::pair<int, int>(info->cia[i], info->destIndexList[i]));
        }

        //for (int i = info->rpa[vertex_index_start]; i < info->rpa[vertex_index_end];) {
        auto it = edge_list[p].cbegin();
        for (int sp = 0; sp < SUB_PARTITION_NUM; sp++) {
            int max = 0;
            int min = 0x7fffffff;

            int num_e = 0;
            for (int ii = 0; ii < info->chunkProp[p][sp].edgeNumChunk; ii++) {
                if (info->rpa[vertex_index_end] <= (sp * info->chunkProp[p][sp].edgeNumChunk + ii + info->rpa[vertex_index_start])) {
                    info->chunkEdgeData[p][sp][ii * 2] =  info->chunkEdgeData[p][sp][(ii - 1) * 2] ;
                    info->chunkEdgeData[p][sp][ii * 2 + 1] = ENDFLAG - 1; // GS will not process this edge, just for alignment;

                } else {
                    int src = (*it).first;
                    info->chunkEdgeData[p][sp][ii * 2] = (*it).first; // source vertex
                    info->chunkEdgeData[p][sp][ii * 2 + 1] = (*it).second; // dest vertex

                    max = (src > max) ? src : max;
                    min = (src > min) ? min : src;

                    it++;
                }

                num_e ++;
            }
            info->chunkProp[p][sp].edgeNumChunk = num_e;
            info->chunkProp[p][sp].srcVertexNumChunk = max - min;
            std::cout << "\tsub: " << sp
                      << " n src_v: " << info->chunkProp[p][sp].srcVertexNumChunk
                      << ", ne: " << info->chunkProp[p][sp].edgeNumChunk  << " Done" << std::endl;
        }
        //}
        std::cout << " Partition " << p << " index range: " << vertex_index_start << "," << vertex_index_end << " Done" << std::endl;

        std::multimap<int, int>().swap(edge_list[p]); // free memory
    }

    std::vector<std::multimap<int, int>>().swap(edge_list); // free memory

    // ===============  Print information  ================
    // for (int p = 6; p < info->partitionNum; p++) {
    //     for (int sp = 0; sp < SUB_PARTITION_NUM; sp++) {
    //         for (int ii = 0; ii < info->chunkProp[p][sp].edgeNumChunk * 2; ii++) {

    //             std::cout << "E[" << p << "][" << sp << "][" << ii << "]:" << info->chunkEdgeData[p][sp][ii] << " ";
    //             if ((ii+1) % 2 == 0) std::cout << std::endl;
    //             // if ((ii % 2 == 1) && (info->chunkEdgeData[p][sp][ii] == 1)) {
    //             //     std::cout << "edge :"<< info->chunkEdgeData[p][sp][ii - 1] <<" "<< info->chunkEdgeData[p][sp][ii]<<" ";
    //             //     std::cout << "prop :"<< info->chunkPropData[sp][info->chunkEdgeData[p][sp][ii - 1]];
    //             //     std::cout << std::endl;
    //             // }
    //             // std::cout << "["<<p<<"]["<<sp<<"]["<<ii<<"]:"<<info->chunkEdgeData[p][sp][ii]<<" ";
    //             // if ((ii + 1) % 2 == 0) std::cout<<std::endl;
    //         }
    //     }
    // }

    // for (int sp = 0; sp < SUB_PARTITION_NUM; sp++) {
    //     for (int i = 0; i < info->alignedCompressedVertexNum; i++) {
    //         std::cout << "[" << i <<"] "<< info->chunkPropData[sp][i] << " ";
    //         if ((i+1) % 10 == 0) std::cout<<std::endl;
    //     }
    // }

}

int acceleratorDataPreprocess(graphInfo *info)
{
    // schedulerRegister();
    dataPrepareProperty(info);
    partitionFunction(info);
    return 0;
}
