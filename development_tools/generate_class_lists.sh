#! /bin/sh
#
# Given a directory of LOP-files, classify them and create lists for
# each class in the current directory.
#


if [ "$1" ]; then
    touch CLASS_1;
    rm CLASS_*;
    dir="$1"
    cl="$*"
    shift
    for file in $dir/*.tptp ; do
        base=`basename $file`
	echo $base
	echo $base >>  CLASS_`classify_problem $* $file |grep " : "| cut -d\( -f2-|cut -d: -f2-| sed -e's/ //'g `
    done
    echo generate_class_lists.sh $cl > description
else
    echo "Usage: generate_class_lists.sh <problem_dir> <classify_problem-options>"
fi
