#!/usr/bin/env python2.7
# ----------------------------------
#
# generate_testfiles.py
#
# Usage: generate_testfiles.py <selector> <optionfile> <basefile>
#
# New generation script for test runs. Create all variants of basefile
# that match  selector in option file.
#
# Format for optionfile is:
#
# <comment>|<rule>*
# <comment> = "# .*\n"
# <rule> = <selector> <suffix> <options> "\n"
#
# where <selector> and <suffix> are arbitrary strings (but suffix
# should avoid troublesome characters) and <options> is a list of
# options to be appended to the options line.
#

import sys
import os
import re
import string


argc = len(sys.argv)
if argc!=4:
    raise RuntimeError, "Usage: generate_testfiles.py <selector> <optionfile> <basefile>"

white_space     = re.compile('\s+')
trail_space     = re.compile('\s*$')
lead_space      = re.compile('^\s*')
empty_line      = re.compile('^\s*$')
todo            = {}

select = re.compile(sys.argv[1])

p=open(sys.argv[2],'r')

l=p.readline()
while l:
    if l[0]=="#" or empty_line.match(l):
        l=p.readline()
        continue

    res = lead_space.sub("", l)
    res = trail_space.sub("", res)
    parts = white_space.split(res,2)
    if len(parts)!=3:
        raise RuntimeError, "Invalid syntax: '"+l+"'"
    if select.search(parts[0]):
        todo[parts[1]]=parts[2]    
    l=p.readline()

p.close()

p = open(sys.argv[3],'r')

execu = re.compile('^Executable: ')
logs  = re.compile('^Logfile: ')
probs = re.compile('^Problemdir: ')
args  = re.compile('^Arguments: ')
timel = re.compile('^Time limit: ')
incl  = re.compile('^Include: ')

rest  = ""
inc   = ""

l=p.readline()
while l:
    if execu.search(l):
        exe = l
    elif logs.search(l):
        log = l
    elif probs.search(l):
        prob = l
    elif args.search(l):
        arg = l
    elif timel.search(l):
        time = l
    elif incl.search(l):
        inc = inc+l
    else:
        rest = rest+l
        
    l=p.readline()

p.close()



arg = trail_space.sub("", arg)
log = trail_space.sub("", log)

logdesc = re.compile('^Logfile: +(/.+)*/protokoll_.+[0-9][0-9][0-9]$')

if not logdesc.search(log):
    raise RuntimeError, "Logfile name malformed!"

for i in todo.keys():
    newspecfile = sys.argv[3]+"_"+i
    if os.path.exists(newspecfile):
        print "File "+newspecfile+" exists, I'm leaving it untouched"
    else:
        print "Generating "+newspecfile 
        p=open(newspecfile, "w")
        p.write(exe)
        p.write(log+"_"+i+"\n")
        p.write(prob)
        p.write(arg+" "+todo[i]+"\n")
        p.write(time)
        p.write(inc)
        p.write(rest)
        p.close()
        
