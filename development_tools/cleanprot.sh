#! /bin/sh
#
# Usage: cleanprot.sh <prot1> ...
#
# Remove maxmem and unknown entries from a protokoll file - these are
# usually bogus and caused by the unreliable and changing runtime
# environment on our sunhalle hosts. 

runcmd="add "
for file in $* ; do
    echo Cleaning $file
   grep -v maxmem $file | grep -v unknown > tmpXXXfile; mv tmpXXXfile $file
   runcmd=`echo $runcmd " " $file`
done
echo $runcmd

