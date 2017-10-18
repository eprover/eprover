#!/usr/bin/env python
# ----------------------------------
"""
Module pylib_io

Functions for handling io, in particular to transparently support
the use of - for stdin/stdout.
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
import os.path
import re
import select
import getopt

NoStdRWStreamException = Exception("You cannot open '-' for both reading and writing!")
UsageErrorException = Exception("Usage Error")

whitespace_re = re.compile('\s+')

Verbose = 0
"""
Verbose level.
"""

def verbout(str, level=1):
    """
    Print messages in verbose mode.
    """
    if level <= Verbose:
        print str


class ECconfigSyntaxError(Exception):
    """
    Exceptions for configuration file errors.
    """
    def __init__(self, error ="Configuration syntax error", source =
                 None, lineno = None):
        self.error  = error
        self.source = source
        self.lineno = lineno

        errstr = ""
        if source:
            errstr = errstr+source+":"
        if lineno != None:
            errstr = errstr+str(lineno)+":"
        errstr = errstr+error

        Exception.__init__(self, errstr)


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
    name = os.path.expanduser(name)
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


def closerange(fd_low, fd_high):
    """
    Close a range of fds. This is a back-port of the eponymous
    function in os for version 2.6.
    """
    for fd in xrange(fd_low, fd_high):
        try:
            os.close(fd)
        except OSError:
            pass



def check_argc(argmin,argmax=None,argv=sys.argv[1:]):
    """
    Print if the required number of arguments has been given, print an
    error message, if not.
    """
    if (argmin and (len(argv)<argmin)) or (argmax and (len(argv)>argmax)):
        if(argmin==1):
            sys.stderr.write("Usage: "+sys.argv[0]+" <arg>\n")
        elif(argmin==2):
            print "Usage: "+sys.argv[0]+" <arg1> <arg2>"
        elif argmax==argmin:
            sys.stderr.write("Usage: " + sys.argv[0]+
                             " <arg1> ... <arg" + repr(argmin) + ">\n")
        elif argmax:
            sys.stderr.write("Usage: " + sys.argv[0]+
                             " <arg1> ... <arg" + repr(argmin) +
                             "> [... <arg" + repr[argmax] + ">]\n")
        else:
            sys.stderr.write("Usage: " + sys.argv[0]+
                             " <arg1> ... <arg" + repr(argmin) + ">...\n")    
        raise UsageErrorException;


def clean_list(l):
    """
    Given a list of strings, return the list stripped, and with empty
    lines and comment lines (starting in #) removed.
    """
    res = map(lambda x:x.strip(), l)
    res = filter(lambda x:not ((x.startswith("#") or x=="")), res);
    return res


def read_real_lines(fp):
    """
    As fp.readlines(), but strip newlines, empty lines, comment lines.
    """
    tmp = fp.readlines()
    return clean_list(tmp)


def parse_lines(name):
    """
    Read a file and return a list of stripped, non-comment, nonempty
    lines.
    """
    fp = flexopen(name, "r")
    res = read_real_lines(fp)
    flexclose(fp)

    return res

def decode_wait_status(status):
    """
    Given a 16 bit status as returned by wait, decode it into a tuple
    (status, signal, core).
    """
    signal = status % 128
    exit_status = status / 256
    if status%256/256 == 1:
        core = True
    else:
        core = False

    return (status, signal, core)

def get_homedir():
    """
    Return the best guess for the users home directory.
    """
    homedir = os.getenv("HOME")
    if not homedir:
        homedir = os.getcwd()

    return homedir


def run_shell_command(cmd):
    """
    Run a local shell command and return the output (or None if an
    error occured).
    """
    fp = os.popen(cmd)
    res = fp.readlines()
    status = fp.close()
    if status:
        print "# Warning: '"+cmd+"' returned status "+\
              repr(decode_wait_status(status)[0])

    return res

def get_load_info():
    """
    Check if an interactive user is logged on to the console and check
    the current load. Return tuple (User, Load), where user is the
    name of the interactive oder or None, and Load is the 5 minute
    average Load.
    """
    who = run_shell_command("w")
    try:
        tmp = who[0].split(",")
        tmp = tmp[3].split()
        load = float(tmp[3])
    except (IndexError, ValueError):
        load = -1.0

    user = None
    for i in who[2:]:
        tmp = i.split()
        try:
            if tmp[1] == "console" or tmp[2] == ":0" or tmp[1]==":0":
                user = tmp[0]
        except IndexError:
            pass

    return (user, load)       
    

def read_will_block(fdobj):
    """
    Determine if the attempt to read from fdobj will result in
    blocking. fdobj needs to a valid file descriptor or have a method
    fileno() that returns one, as per select.select(). 
    """
    ready = select.select([fdobj], [], [])
    return not fdobj in ready[0]


def write_will_block(fdobj):
    """
    Determine if the attempt to write to fdobj will result in
    blocking. fdobj needs to a valid file descriptor or have a method
    fileno() that returns one, as per select.select(). 
    """
    try:
        ready = select.select([], [fdobj], [])
    except ValueError:
        return False
    return not fdobj in ready[1]


def parse_config_string(configstr, sourcename=None, sep=":",
                        allowed_keys=None, required_keys=None):
    """
    Parse a UNIX-style configuration with #-comments and
    key-value pairs separated by a fixed string, represented by a
    single string.

    \param configstr      is the actual configuration text,
                          interpreted as a sequence of newline-
                          separated lines.
    \param sourcename     is a descriptive name of the source, used
                          for error messages.
    \param sep            is a string seperating key and value.
    \param allowed_keys   describes the list of keys that should be
                          acepted. If None, all keys are accepted.
    \param required_keys  describes the list of required keys. If
                          None, no keys are required.
    \return               Dictionary of key/value association and list
                          of key/value pairs.
    """
    if not sourcename:
        sourcename = "Configuration string starting with "+inpstr[:20]

    result = []
    keys   = {}
    config_list = configstr.split("\n")
    lineno = 0
    for line in config_list:
        lineno = lineno+1
        comment = line.split("#")
        line = comment[0]
        line = line.strip()
        if not line:
            continue

        try:
            (key, value) = line.split(sep, 1)
        except ValueError:
            raise ECconfigSyntaxError("Malformed line '"+line+"'",
                                      sourcename, lineno)
        key = key.strip()
        if allowed_keys != None and not key in allowed_keys:
            raise ECconfigSyntaxError("Unknown key '"+key+"'",
                                     sourcename, lineno)
        value = value.strip()
        result.append((key, value))
        keys[key] = value
        
    if required_keys:
        for i in required_keys:
            if not key in keys:
                raise EConfigSyntaxError("Missing key '"+i+"'",
                                         sourcename)

    return (keys, result)



def parse_config_file(source, sep=":", allowed_keys=None, required_keys=None):
    """
    Parse a UNIX-style configuration file with #-comments and
    key-value pairs separated by a fixed string.

    \param source         is either a string (interpreted as a file
                          name) or a file pointer.
    \param sep            is a string seperating key and value.
    \param allowed_keys   describes the list of keys that should be
                          acepted. If None, all keys are accepted.
    \param required_keys  describes the list of required keys. If
                          None, no keys are required.
    \return               Dictionary of key/value pairs.
    """

    if type(source) == type("str"):
        fp = open(source)
        inpstr = fp.read()
        fp.close()
        sourcename = source
    else:
        assert(type(source)==type(sys.stdin))
        inpstr = fp.read()
        sourcename = source.name

    return parse_config_string(inpstr, sourcename, sep, allowed_keys, required_keys)

def get_directory(filename):
    """
    Given a filename, return the directory that file is in.
    """
    if filename == "-":
        return os.getcwd()
    return os.path.dirname(os.path.abspath(os.path.expanduser(filename)))
        
        
if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)

    print get_load_info()
    
