#!/bin/sh

cat /dev/null > ../problem_features_tptp_4.0.1

for file in `ls|grep \.p`; do
    echo $file
    ulimit -t 500
    classify_problem --tstp-in -caaaaaaaaaaaaa $file>> ../problem_features_tptp_4.0.1
done
