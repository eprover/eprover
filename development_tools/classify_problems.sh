#!/bin/sh

# Delete this if only missing problems are added!
cat /dev/null > ../problem_features_7.2.0_raw

for file in `ls|grep [_+=-].*\.p`; do
    echo "Processing " $file
    # for file in `cat ../missing`; do
    ProblemSPC=`grep " SPC " $file | sed -e "s/.* : //"`
    # echo $ProblemSPC
    if  [ `expr "$ProblemSPC" : "TF0.*"` != 0 ]  || [ `expr "$ProblemSPC" : "FOF.*"` != 0 ] || [ `expr "$ProblemSPC" : "CNF.*"` != 0 ]  ; then
       # echo $file
       ulimit -t 5000
       classify_problem --tstp-in $file>> ../problem_features_7.2.0_raw
    else
           echo "Skipping " $file
    fi
done
