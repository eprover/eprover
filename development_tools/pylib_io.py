#!/usr/bin/env python2.2
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
    """
    Open a file or stdin/out for reading. Uses the convention that a
    dash represents stdin/out.
    """
    if(name == "-"):
        if(mode == "r"):
            return sys.stdin;
        elif(mode == "w"):
            return sys.stdout
        else:
            raise NoStdRWStreamException
    return open(name, mode)

def flexclose(file):
    """
    "Closes" a file object. If stdin/out, flushes it, otherwise  calls
    close on it.
    """
    if((file == sys.stdout) or (file == sys.stderr)):
        file.flush()
    else:
        file.close()

def check_argc(argmin,argv=sys.argv):
    """
    Print if the required number of arguments has been given, print an
    error message, if not.
    """
    if((len(argv)-1)<argmin):
       if(argmin==1):
           sys.stderr.write("Usage: "+sys.argv[0]+" <arg>\n")
       elif(argmin==2):
           print "Usage: "+sys.argv[0]+" <arg1> <arg2>"
       else:
           sys.stderr.write("Usage: " + sys.argv[0]+
                            " <arg1> ... <arg" + repr(argmin) + ">\n")
       raise UsageErrorException
                            
def get_options(argv=sys.argv[1:]):
    """
    Filter argument list for arguments starting with a -.
    """
    options = filter(lambda x:x[0:1]=="-", argv)
    return options

def get_args(argv=sys.argv[1:]):
    """
    Filter argument list for real arguments.
    """
    files   = filter(lambda x:x[0:1]!="-", argv)
    return files


           
        
