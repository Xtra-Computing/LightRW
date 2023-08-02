#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <random>

#include <unistd.h>

#include "pcg_basic.h"

int main(int argc, char **argv)
{
    srand( (unsigned)time(NULL) );
    pcg32_srandom(42u, 54u);
    for (int i = 0; i < rand();i ++)
    {
        pcg32_random();
    }
    uint32_t degree = 3;
    for (int i = 0; i < 10 ; i++)
    {
        static const double inv_rand_max = 1.0/((double)0xffffffffu + 1.0);

        //min +  (int) (((double) rand() / (RAND_MAX + 1.0)) * (max - min + 1))
        uint32_t rn = pcg32_random();
        uint32_t sampled_index =   (uint32_t)(((double)(degree ))*rn*inv_rand_max);
        //(uint32_t) (((double) rn / (0xffffffffu + 1.0)) * (degree));
        printf("%u \n",sampled_index);
    }

    return 0;
}