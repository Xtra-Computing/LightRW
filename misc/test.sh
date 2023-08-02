#!/bin/bash


log_file=real_graph_test.log

run(){
./$1.app build_dir_$1/kernel.xclbin /data/graph_dataset/$2 | tee -a ${log_file}
}

app=$1

run ${app} com-youtube.txt
run ${app} cit-Patents.txt
run ${app} LiveJournal1.txt
run ${app} orkut.txt
run ${app} web-uk-2002-all.mtx
