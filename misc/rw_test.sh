#!/bin/bash


if [ $# -lt 1 ];
  then
    echo "Usage: "
    echo "rw_test.sh <app_name> <path of all dataset> "
    echo "eg.: ./misc/rw_test.sh  metapath_x4  /data/graph_at_scale/  "
    exit -1
fi


app=$1

if [ $# -lt 2 ];
  then
    echo "Use default path of datasets: /data/graph_at_scale/dataset"
    dataset_path="/data/graph_at_scale/dataset"
else
    dataset_path=$2
fi


DATASET=(   'amazon' \
            'youtube' \
            'patents' \
            'orkut' \
            'uk2002' \
)

__run(){
    graphapp_name=$1
    dataset_dir=$2
    dataset_name=$3
    log_dir=$4


    ./${graphapp_name}.app -graph ${dataset_dir}/${dataset_name}  2>&1 | tee ${log_dir}/log_${graphapp_name}_${dataset_name}.log
}

time_string=`date +%Y_%m_%d_%H_%M_%S`

log_dir="log_${app}_${time_string}"
echo ${log_dir}
mkdir -p ${log_dir}


for dataset  in "${DATASET[@]}"
do
    echo "$dataset"
    #__run  $app $dataset_path  ${dataset} ${log_dir}

done