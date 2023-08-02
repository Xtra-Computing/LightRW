#include "host_graph_api.h"

#define INT2FLOAT                   (pow(2,30))

int float2int(float a) {
    return (int)(a * INT2FLOAT);
}

float int2float(int a) {
    return ((float)a / INT2FLOAT);
}

unsigned int dataPrepareGetArg(graphInfo *info)
{
    return float2int((1.0f - kDamp) / info->vertexNum);
}

int dataPrepareProperty(graphInfo *info)
{
    float init_score_float = 1.0f / (info->vertexNum);
    int init_score_int = float2int(init_score_float);

    for (int sp = 0; sp < SUB_PARTITION_NUM; sp++) {
        for (int i = 0; i < info->alignedCompressedVertexNum; i++) {
            if (i >= info->compressedVertexNum) {
                info->chunkPropData[sp][i] = 0;
            } else {
                info->chunkOutDegData[i] = info->outDeg[i];
                info->chunkPropData[sp][i] = i;
            }
        }
    }

    // for (int i = 0; i < info->vertexNum; i++) {
    //     std::cout << "[" <<i<<"]:" <<info->vertexPushinProp[i]<<" ";
    //     if ( i % 50 == 0) std::cout << std::endl;
    // }

    std::cout << "[INFO] Data prepare property done " << std::endl;
    return 0;
}