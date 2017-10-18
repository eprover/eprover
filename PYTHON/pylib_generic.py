#!/usr/bin/env python
# ----------------------------------
"""
Module pylib_generic

Generic stuff useful in many different contexts.

Copyright 2003,2004 Stephan Schulz, schulz@eprover.org

This code is part of the support structure for the equational
theorem prover E. Visit

 http://www.eprover.org

for more information.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program ; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston,
MA  02111-1307 USA 

The original copyright holder can be contacted as

Stephan Schulz (I4)
Technische Universitaet Muenchen
Institut fuer Informatik
Boltzmannstrasse 3
Garching bei Muenchen
Germany

or via email (address above).
"""


import sys
import os
import re
import time


whitespace_re = re.compile('\s+')
"""Regular expression representing white space."""

dstring_re     = re.compile('".*?"')
"""Regular expression representing a string in double quotes."""

dqstring_re     = re.compile('".*?[^\\\\]"')
"""Regular expression representing a string in double quotes with quoting."""

sstring_re     = re.compile("'.*?'")
"""Regular expression representing a string in single quotes."""

sqstring_re     = re.compile("'.*?[^\\\\]'")
"""Regular expression representing a string in single quotes with quoting."""

other_re       = re.compile("[^'\"\s]+")
"""
Regular expression representing anything not containing white space or
quotes.
""" 

sqquote_re = re.compile("[^\\\\]'")
"""Regular expression represening  an unquoted single quote."""

nonprint_ascii_re = re.compile("[^ -\177]")
"""
Regular expression representing all non-printable characters, where
only ASCII 32-127 are considered printable.
"""

def asciify_str(str, repl="_"):
    """
    Return str with all non-printable characters replaced according to
    repl (which can be a strig or a function as per re.sub()).
    """
    res = nonprint_ascii_re.sub(repl, str)
    return res


def break_shell_str(opts):
    """
    Break a string of arguments into an array of argments as the shell would.
    """
    res = []
    tmp = ""
    opts = opts.strip()+" "
    while opts != "":
        mo = whitespace_re.match(opts)
        if mo:
            res.append(tmp)
            tmp = ""
            opts = opts[mo.end():]
        else:
            mo = dstring_re.match(opts) or sstring_re.match(opts)
            if mo:
                tmp = tmp+mo.group()[1:-1]
                opts = opts[mo.end():]
            else:
                mo = other_re.match(opts)
                try:
                    tmp = tmp+mo.group()
                    opts = opts[mo.end():]
                except AttributeError:
                    raise Error("Impossible string in break_shell_str()");
    return res   


class timer(object):
    def __init__(self, settime):
        self.set(settime)

    def remaining(self):
        now = time.time()
        if now > self.expiry:
            return 0
        return self.expiry - now

    def expired(self):
        return self.remaining() == 0


    def set(self, settime):
        self.expiry = time.time()+settime

    def __str__(self):
        return "<timer: %f (%f remaining)>"%(self.expiry, self.remaining())


if __name__ == '__main__':

    tmp = """
    eprover -xAuto -tAuto -H"(1* Error, 3* Fridolin)"
    """
    print break_shell_str(tmp)
    

