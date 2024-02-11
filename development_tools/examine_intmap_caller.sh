#!/bin/bash

#dir_array=($(ls -d ../../Code/TPTP-v8.2.0/Problems/*/))
dir_array=($(ls -d ../EXAMPLE_PROBLEMS/*/))

for i in "${dir_array[@]}"; do
    sleep 1 #2700
    echo "$i"
    nohup python3 examine_intmap.py $i ../../csv_files s
done


