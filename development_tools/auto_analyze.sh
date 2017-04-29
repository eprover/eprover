#!/bin/sh

grep CLASS $* | grep proto | gawk '{array[$4] =array[$4]+$5}END{for(i in array)printf("%-56s %4ld\n", i ,array[i])}' | sort -n -r -k2
