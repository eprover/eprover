#!/bin/sh

# Delete this if only missing problems are added!
cat /dev/null > ../problem_features_6.4.0_raw

for file in `ls|grep [+-].*\.p`; do
# for file in `cat ../missing`; do
    echo $file
    ulimit -t 5000
    classify_problem --tstp-in --free-numbers $file>> ../problem_features_6.4.0_raw
done
