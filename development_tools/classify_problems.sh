#!/bin/sh

cat /dev/null > ../problem_features
cat /dev/null > ../problem_features_npp

for file in `ls|grep tptp`; do
    echo $file
    classify_problem --tptp-in -caaaaaaaaaa --ax-some-limit=20 --ax-many-limit=100 --lit-some-limit=15 --lit-many-limit=100 --term-some-limit=60 --term-many-limit=1000 $file>> ../problem_features
    classify_problem --tptp-in -caaaaaaaaaa --ax-some-limit=20 --ax-many-limit=100 --lit-some-limit=15 --lit-many-limit=100 --term-some-limit=60 --term-many-limit=1000 $file >> ../problem_features_npp
done
