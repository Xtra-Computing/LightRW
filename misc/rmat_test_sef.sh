#!/bin/bash

ef=$2
log_file=rmat_test_${ef}.log

run(){
./$1.app build_dir_$1/kernel.xclbin /data/graph_dataset/rmat/$2 | tee -a ${log_file}
}


for para_list in {12..21}
do
    run $1 rmat-${para_list}-${ef}.txt
    echo "next para"
  #wait

done