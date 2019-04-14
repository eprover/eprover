#! /bin/sh
#
# Given a directory of LOP-files, classify them and create lists for
# each class in the current directory.
#


if [ "$1" ]; then
    touch CLASS_1;
    rm CLASS_*;
    classify_problem -p $*  | gawk 'BEGIN{FS=" : "}{cl[$3] = cl[$3] $1 "\n"}END{for(i in cl){printf cl[i] > "CLASS_F" i}}'
    echo generate_class_lists_from_file.sh $* > description
else
    echo "Usage: generate_class_lists_from_file.sh <feature_file> <classify_problem-options>"
fi
