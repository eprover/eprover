#! /bin/sh

time=""
optflag="-t"

if [ "$1" = "-t" ]; then
    shift
    time=$1
    shift
fi

if [ "$1" ]; then
    for file in $* ; do
	printf "%-65s%s\n" $file "`eauswert.awk $file 1000000 $time |cut -d\: -f2-`"
    done
else
    for file in proto* ; do
	printf "%-65s%s\n" $file "`eauswert.awk $file 1000000 $time |cut -d\: -f2-`"
    done
 fi
