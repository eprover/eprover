#!/bin/sh

cat /dev/null > ../problem_features_tcnf16_new

for file in `ls|grep \.p`; do
    echo $file
    ulimit -t 100
    classify_problem --tstp-in -caaaaaaaaaaaaa $file>> ../problem_features
done
