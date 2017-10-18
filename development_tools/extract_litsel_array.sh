#!/bin/sh

egrep '   ".?Select[a-zA-Z0-9]*",|   "NoSelection",|   "NoGeneration",' che_litselection.c | sed -e 's/,//' | awk '{printf "%-40s:\n", $0}'>bla1.tmp

egrep '^   .?Select[a-zA-Z0-9]*,' che_litselection.c | sed -e 's/,/",/' | awk '{print "\"" $1}'>bla2.tmp

paste -d" " bla1.tmp bla2.tmp
rm bla1.tmp bla2.tmp


