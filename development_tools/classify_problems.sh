#!/bin/sh

cat /dev/null > ../problem_features
cat /dev/null > ../problem_features_npp

for file in `ls|grep \.p`; do
    echo $file
    ulimit -t 100
    classify_problem --tstp-in -caaaaaaaaaaaa --ax-some-limit=20 --ax-many-limit=100 --lit-some-limit=15 --lit-many-limit=100 --term-medium-limit=60 --term-large-limit=1000 $file>> ../problem_features
    classify_problem --tstp-in -caaaaaaaaaaaa --ax-some-limit=20 --ax-many-limit=100 --lit-some-limit=15 --lit-many-limit=100 --term-medium-limit=60 --term-large-limit=1000 $file >> ../problem_features_npp
done
