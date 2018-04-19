#!/usr/bin/env python3

"""
Usage: cleanprot.py eprot1 ... eprotn

Read multiple E protocol files and collate them. Comments are taken
from the first file, results from all files, with success overriding
unknown. "Short" entries (of failures) will be padded to consistent
length based on the first success found. This is a hack and only
tested for a few well-behaved use-cases!
"""

import sys
import re

comment_re = re.compile("^#.*")
sd_comment_re = re.compile("^# [0-9] ")
spec_comment_re = re.compile("^#[ ./]*eprover")

spec     = {}       # eprover ... goes first
fields   = set([])  #  1 Field descriptions go second
comments = {}  # all other comments

results = {}
maxfields = 0
template  = []

def fixcomment(comment):
    comps = comment.split(None,2)
    return "{} {:>2} {}".format(*comps)

for file in sys.argv[1:]:
    fp = open(file, "r")

    for line in fp:
        if comment_re.match(line):
            # print("Adding comment", line, end="")
            if sd_comment_re.match(line):
                fields.add(fixcomment(line))
            elif spec_comment_re.match(line):
                c_key = line[:10]
                if not c_key in spec:
                    spec[c_key] = line
            else:
                c_key = line[:10]
                if not c_key in comments:
                    comments[c_key] = line

        else:
            parts = line.split()
            if len(parts) > maxfields:
                maxfields = len(parts)
                template = parts
            key = parts[0]
            if not key in results:
                results[key] = line, parts
            else:
                oldline, oldparts = results[key]
                # print(oldparts[3])
                if oldparts[3] == "unknown":
                    results[key] = line, parts

    fp.close()

for i in sorted(spec.keys()):
    print(spec[i], end="")
if fields:
    print("".join(sorted(fields)), end="")
for i in sorted(comments.keys()):
    print(comments[i], end="")


for i in sorted(results.keys()):
    if len(results[i][1])<maxfields:
        padding = ["            -" * (maxfields - len(results[i][1]))]
        padstr  = " ".join(padding)
        results[i] = results[i][0].strip()+padstr+"\n",  results[i][1]+padding

for i in sorted(results.keys()):
    print(results[i][0], end="")
