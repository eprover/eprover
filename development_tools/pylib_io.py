#! /usr/bin/env python
# ----------------------------------
#
# Module pylib_io
#
# Functions for handling io, in particular to transparently support
# the use of - for stdin/stdout.

import sys

NoStdRWStreamException = "You cannot open '-' for both reading and writing!"
UsageErrorException = "Usage Error"

def flexopen(name, mode):
    if(name == "-"):
        if(mode == "r"):
            return sys.stdin;
        elif(mode == "w"):
            return sys.stdout
        else:
            raise NoStdRWStreamException
    return open(name, mode)

def flexclose(file):
    if((file == sys.stdout) or (file == sys.stderr)):
        file.flush()
    else:
        file.close()

def check_argc(argmin,argv=None):
    if(argv==None):
        argv=sys.argv
    if((len(argv)-1)<argmin):
       if(argmin==1):
           print "Usage: "+sys.argv[0]+" <arg>"
       elif(argmin==2):
           print "Usage: "+sys.argv[0]+" <arg1> <arg2>"
       else:
           print "Usage: "+sys.argv[0]+" <arg1> ... <arg" +repr(argmin)+">"
       raise UsageErrorException


           
        
