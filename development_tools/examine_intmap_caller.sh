#!/bin/bash

dir_array=($(ls -d ../../TPTP-v8.2.0/Problems/*/))
# dir_array=($(ls -d ../EXAMPLE_PROBLEMS/*/))

for i in "${dir_array[@]}"; do
    echo "$i"
    nohup python3 examine_intmap.py $i ../../csv_files f &
    sleep 120
done


#  cat hard_problems_list.txt | grep -v '\^'
