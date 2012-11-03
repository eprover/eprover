#!/bin/sh

cat /dev/null > ../problem_features_5.4.0_raw

for file in `ls|grep \.p`; do
    echo $file
    ulimit -t 500
    classify_problem -r --tstp-in $file>> ../problem_features_5.4.0_raw

done
