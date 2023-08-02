#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>

#include "thundring.h"

#define NUM_PE (1)

int main(int argc, char *argv[]) {

    cl_accelerator acc(argc, argv);
    auto q = acc.alloc_task_queue();

    test_store   krnl_test_store;

    krnl_test_store = test_store(acc, 0, 1024* 1024*64);
    krnl_test_store.init();
    for(int i =0; i < 10 ; i++)
    {
        krnl_test_store.exec();
    }
}

