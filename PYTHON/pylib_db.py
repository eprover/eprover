#!/usr/bin/env python
# ----------------------------------
"""
Module pylib_db

Functions for storing semi-persitent simple associations
backed by a file. The current implementation is straightforward and
simple.

Copyright 2011 Stephan Schulz, schulz@eprover.org

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


import os
import os.path
import tempfile
import sys
import getopt
import pylib_io


class key_db(object):
    """
    Manage a set of keys with associated (ordered) list of string items.
    """
    def __init__(self, file_store=None):
        """
        Initialize key DB. If name is given, try to read persisted
        state from the file.
        """
        self.data = {}
        self.file_store = None
        if file_store:
            self.file_store = os.path.abspath(file_store)
            self.db_dir = os.path.dirname(self.file_store)
            
            self.load(self.file_store)
                
    def add_entry(self, key, data):
        """
        Add a key/value(list) pair to the DB.
        """
        self.data[key] = data

    def find_entry(self, key):
        """
        Return the data associated with key, or None if key does not
        exist. 
        """
        try:
            return self.data[key]
        except KeyError:
            return None

    def del_entry(self, key):
        """
        Delete an entry from the db. Return True on success, or False
        if entry did not exist in the first place.
        """
        try:
            del self.data[key]
            return True
        except KeyError:
            return False

    def sync(self):
        """
        If a backup file exists, write DB content (safely) to the
        file. 
        """
        if not self.file_store:
            return
        #fp = tempfile.NamedTemporaryFile("wb", -1, ".db",
        #                                      "dbtmp_", self.db_dir,
        #                                      False)
        fp = open(self.db_dir+"/dbtmp_017.db", "wb")
        tmpfile = fp.name
        try:
            self.write_db(fp)
            fp.close()
            #print "rename", tmpfile, "->", self.file_store
            os.rename(tmpfile, self.file_store)
        except OSError:
            pass

            
    def load(self, file):
        """
        Try to read the DB from file. Return True on success, False
        otherwise. 
        """
        try:
            fp = open(file, "r")
            self.read_db(fp)
            fp.close()
            return True
        except IOError:
            return False        

    def read_db(self, fp):
        """
        Read a DB from the provided file object. Return True on
        success, False otherwise. On failure, the DB is unchanged. 
        """
        try:
            tmp = eval(fp.read(), {"__builtins__":None},{})
        except:
            return False
        self.data = tmp
        return True

    def write_db(self, fp):
        """
        Write a DB to the provided file object.
        """
        try:
            fp.write(repr(self.data))
            return True
        except:
            return False

        
if __name__ == '__main__':
    opts, args = getopt.gnu_getopt(sys.argv[1:], "h")

    for option, optarg in opts:
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)
            
    db = key_db("test.db")
    db.add_entry("a", ["b", "c"])
    db.add_entry("d", ["e", "f"])
    db.sync()
    db = None
    
    newdb = key_db("test.db")
    print newdb.find_entry("a")
    os.unlink("test.db")
