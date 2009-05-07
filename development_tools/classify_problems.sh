#!/bin/sh

cat /dev/null > ../problem_features_tptp_3.5.0

for file in `ls|grep \.p`; do
    echo $file
    ulimit -t 300
    classify_problem --tstp-in -caaaaaaaaaaaaa $file>> ../problem_features_tptp_3.5.0
done
