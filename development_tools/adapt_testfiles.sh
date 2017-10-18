#! /bin/sh
#
# adapt_testfiles.sh <files>
#
# Copyright 2002 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# Adapt all files from the command line to the current test 
# environment. This probably only is useful on my local setup.
#

for file in $* ; do
    echo Processing $file
    tmpname=adapt_$$_`hostname`
    adapt_testfile.awk $file > $tmpname
    mv $tmpname $file
done



