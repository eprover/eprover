#! /bin/sh

if [ "$1" ]; then
    for file in $* ; do
	printf "%-48s%s\n" $file "`eauswert.awk $file|cut -d\: -f2-`"
    done
else
    for file in proto* ; do
	printf "%-48s%s\n" $file "`eauswert.awk $file|cut -d\: -f2-`"
    done
 fi

