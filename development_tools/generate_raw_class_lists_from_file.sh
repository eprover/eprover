#! /bin/sh
#
# Given a directory of LOP-files, classify them and create lists for
# each class in the current directory.
#


if [ "$1" ]; then
    touch CLASS_1;
    rm CLASS_*;   
    classify_problem -pr $*  | gawk 'BEGIN{FS=" : "}{cl[$3] = cl[$3] $1 "\n"}END{for(i in cl){printf cl[i] > "CLASS_" i}}'
    echo generate_raw_class_lists_from_file.sh $* > description
else
    echo "Usage: generate_raw_class_lists_from_file.sh <problem_dir> <classify_problem-options>"
fi
