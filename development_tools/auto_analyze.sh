#!/bin/sh

grep CLASS $* | grep proto | gawk '{array[$4] =array[$4]+$5}END{for(i in array){printf("%-66s %4ld\n", i ,array[i]);sum+=array[i]};printf("Total solutions: %d\n",sum);}' | sort -n -r -k2
