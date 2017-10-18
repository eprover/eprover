#!/bin/sh

old=`ls -l prot`

while test -f prot; do
    sleep 200;
    new=`ls -l prot`
    if [ "$old" = "$new" ]; then
	kill -HUP `ps -elf | grep schulz | grep ssh | grep sunhalle | grep -v "sh -c" | gawk '{print $4}'`
    else
        old=$new
    fi;
    echo $old
done;

