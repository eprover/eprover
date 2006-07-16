#!/bin/sh

for file in $* ; do
   sequential_eprover.awk $file
done

