#! /usr/bin/env python
# ----------------------------------
#
# Module pylib_eprots
#
# Functions (and classes) for manipulatig E result protocols (this is
# probably fairly specialized and not that useful for others...
#
# Copyright 2003 Stephan Schulz, schulz@informatik.tu-muenchen.de
#
# This code is part of the support structure for the equational
# theorem prover E. Visit
#
#  http://www4.informatik.tu-muenchen.de/~schulz/WORK/eprover.html 
#
# for more information.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program ; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA  02111-1307 USA 
#
# The original copyright holder can be contacted as
#
# Stephan Schulz (I4)
# Technische Universitaet Muenchen
# Institut fuer Informatik
# Boltzmannstrasse 3
# Garching bei Muenchen
# Germany
#
# or via email (address above).

import re
import string


def break_prot_line(line):
    if line[0] == "#":
           return None
    parts = string.split(line, None)
    for i in range(0, len(parts)):
        try:
            p = float(parts[i])
        except ValueError:
            try:
                p = int(parts [i])
            except ValueError:
                p = parts[i]
        parts[i] = p
    return parts            


def get_relevant_parts(line):
    parts = break_prot_line(line)
    if parts:
        return (parts[0],parts[1],parts[2])
    return None

class process_line:
    def __init__(self, round_fun):
        self.round_fun = round_fun;

    def __call__(self,line):
        if line[0] == "#":
           return line
        parts = string.split(line, None, 4)
        return "%-29s %s %8.3f %-10s %85s" % (parts[0], parts[1],
                                            self.round_fun(float(parts[2])),
                                            parts[3],parts[4])


class eprotocol:
    def __init__(self):
        self.name      = "Unknown"
        self.data      = {}
        self.proofs    = 0
        self.models    = 0
        self.successes = 0
        self.succ_time = 0
        self.entries   = 0
        self.comments  = ""

    def insert_line(self, line):
        tmp = get_relevant_parts(line)
        if tmp:
            entry = tmp[0]
            state = tmp[1]
            time  = tmp[2]
            data[entry] = (state, time)
            self.entries += 1
            if state == "T":
                self.proofs    += 1
                self.successes += 1
                self.succ_time += time
            elif state == "N":
                self.models    += 1
                self.successes += 1
                self.succ_time += time
        else:
            self.comments += line

        def parse_prot(self, name):
            f = open(name, "r")
        
