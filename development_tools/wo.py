#! /usr/bin/env python
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

pylib_io.check_argc(2)

setf = pylib_io(sys.argv[1])
wof  = pylib_io(sys.argv[2])


