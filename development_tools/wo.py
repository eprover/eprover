#!/usr/bin/env python2.7
# ----------------------------------
#
# Usage: wo.py <s1> <s2>
#
# Return all lines from <s1> that do not occur in <s2>
#

import sys
import re
import string
import pylib_io

def id(string):
    return string

def firstfield(str):
    parts = string.split(str)
    if(len(parts)>0):
        return parts[0]
    return ""


def without(file_source, file_remove, abstract_fun):
    source = {}
    lines = file_remove.readlines()
    for line in lines:
        source[abstract_fun(line)] = 1;
    lines = file_source.readlines()
    for line in lines:
        if(not (abstract_fun(line) in source)):
            sys.stdout.write(line)

pylib_io.check_argc(2)
file_source = pylib_io.flexopen(sys.argv[1], "r")
file_remove = pylib_io.flexopen(sys.argv[2], "r")

without(file_source, file_remove, id)
