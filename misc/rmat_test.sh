#!/bin/bash

log_file=rmat_test_scan.log

run(){
./$1.app build_dir_$1/kernel.xclbin /data/graph_dataset/$2 | tee -a ${log_file}
}

EDGE_FACTOR_LIST=( '2' \
            '4' \
            '8' \
            '16' \
            '32' \
            '64' \
)


for ef  in "${EDGE_FACTOR_LIST[@]}"
do
    for s in {12..21}
    do
        echo "para" ${s} ${ef}
        run $1 rmat-${s}-${ef}.txt
        echo
      #wait

    done
done