#!/bin/sh

cat /dev/null > ../problem_features_6.4.0_raw

for file in `ls|grep [+-].*\.p`; do
#for file in `cat ../missing`; do
    echo $file
    ulimit -t 5000
    classify_problem --tstp-in $file>> ../problem_features_6.4.0_raw
done
