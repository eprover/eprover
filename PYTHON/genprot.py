#!/usr/bin/env python3

"""
genprot 0.2

Usage: genprot.py <archive_1> ... <archive_n>

Read a list of compressed or unpacked output archives
from the StarExec cluster logic solving service (https://www.starexec.org/)
and convert the data into a protocol format (csv) usable by other tools for performance analyses within E.
For each parameter set a separate protocol file is generated.

Output files to be processed within the archives or paths need to be in the form:
proverversion/problemname/somefile.txt
e.g.
E 1.9.1pre005___E---1.9.1_0007_rw2121/ALG001-1.p/172496804.txt

Options:

-h Print help.

--header     add cvs header with column names
--delimiter  delimiter between values
--default    default value for fields in cvs
--metadata   add metadata from directory structure - file and config and jobarchive name
--compact    do not add alignment whitespace
--features   add feature columns with features of the problem taken from supplied feature file
--verbose    print processed file names

Copyright 2015 Martin Möhrmann, moehrmann@eprover.org,
          2019-23 Stephan Schulz, schulz@eprover.org

This code is part of the support structure for the equational
theorem prover E. Visit

 http://www.eprover.org

for more information.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

The original copyright holders can be contacted as

Stephan Schulz
Duale Hochschule Baden-Württemberg Stuttgart
Rotebühlplatz 41
Raum: 0.01
70178 Stuttgart
Germany

or via email (address above).
"""


import argparse
import tarfile
import zipfile
import sys
import os
import re

from itertools import chain
from os.path import dirname, splitext, basename, isdir, isfile
from collections import defaultdict


protfile = (lambda name: "protocol_" + name + ".csv")

firstkeys   = ["Problem", "Status", "User time", "Failure", "Version", "Preprocessing time"]
metakeys    = ["Filename", "Configname", "Archivename"]
removekeys  = ["eprover", "Command", "Computer", "Model", "CPU", "Memory", "OS", "CPULimit", "DateTime", "CPUTime"]
featurekeys = ["Type", "Equational"]

version_re = re.compile("[0-9]+[.][0-9]+")

failuremap = {"User resource limit exceeded"    :"maxres",
              "Out of unprocessed clauses!" :"incomplete",
              "Resource limit exceeded (memory)":"maxmem",
              "Resource limit exceeded (time)"  :"maxtime",
              "exec failed"         :"starexec",
              "Inappropriate"       :"problemtype",
              "unknown"             :"unknown"}

statusmap = {"unknown"                        :"F",
             "exec failed"                    :"F",
             "SZS status GaveUp"              :"F",
             "SZS status ResourceOut"         :"F",
             "SZS status Satisfiable"         :"N",
             "SZS status CounterSatisfiable"  :"N",
             "SZS status Theorem"             :"T",
             "SZS status ContradictoryAxioms" :"T",
             "SZS status Unsatisfiable"       :"T",
             "SZS status Inappropriate"       :"F",
             "SZS status Error"               :"F"}

def rjust(amount): return lambda s: s.rjust(amount)
def ljust(amount): return lambda s: s.ljust(amount)

adjustmap     = defaultdict(lambda:rjust(12))
adjustmap.update({"User time"         :rjust( 8),
                  "Preprocessing time":rjust( 8),
                  "Status"        :rjust( 1),
                  "Failure"       :rjust(10),
                  "Type"          :rjust( 8),
                  "Config"        :rjust(12),
                  "Filename"          :ljust(12),
                  "Problem"       :ljust( 8)})

def firstvalue(dictionary):
    return next(iter(dictionary.values()))

def clean_value(value):
    value = value.strip()
    if value.endswith(" s"):
        value = value[:-2]
    elif value.endswith(" pages"):
        value = value[:-6]
    elif value.endswith("MB"):
        value = value[:-2]
    return value


def remove_timestamp(line):
    # split prefixed timestamp X.XX/X.XX % or X.XX/X.XX #
    split = line.split("%", 1)
    if len(split) == 2 and len(split[0])<25:
        return split[1].strip()
    else:
        split = line.split("#", 1)
        if len(split) == 2 and len(split[0])<25 and split[0].find('(')==-1:
            return "# "+split[1].strip()
        else:
            # might be: 0.00/0.00   exec failed: No such file or directory
            split = line.split("\t", 1)
            if len(split) == 2 and split[1].startswith("exec failed"):
                return split[1].strip()
            if len(split) == 2 and split[1].startswith("Alarm clock"):
                # print("Found", "# "+ split[1].strip())
                return "# "+ split[1].strip()
            else:
                # line with only timestamp or other data
                return ""


def clean_key(entry, keymap):
    """
    In the latest version of pylib_e_multi, stderr and stdout run
    together, sometimes mixing up lines and obscuring keys. This might
    help...
    """
    for i in keymap.keys():
        if entry.startswith(i):
            return i
    return entry


def make_entry(lines):
    entry = dict()
    for line in lines:
        status = False
        line = line.decode()
        # print(line)
        line = remove_timestamp(line)
        line = line.replace("eprover: CPU time limit exceeded, terminating", "", 1)

        if not line.startswith("#"):
            continue
        else:
            # print(line)            
            line = line[2:]
        # print(line)            
        split = line.split(":", 1)
        key   = split[0].strip()
        # Correct for TPTP errors causing E parse error mistaken for a result
        if(key.startswith("Type mismatch")):
            continue
        if(key.startswith("partial match")):
            continue
        value = clean_value(split[1]) if len(split) == 2 else ""
        if key.startswith("SZS status Inappropriate"):
            return None
        if key.startswith("Schedule exhausted") or key.startswith("Alarm clock"):
            # print("Here", key)
            entry["Status"] = "F"
            entry["Failure"] = "maxtime"
            status = True
        elif not status and key.startswith("SZS status"):
            entry["Status"] = statusmap[clean_key(key, statusmap)]
        elif key.startswith("exec failed"):
            entry["Status"] = statusmap["exec failed"]
            entry["Failure"] = failuremap["exec failed"]
        elif key == "Problem":
            if not value.startswith('%'):
                entry[key] = (value.split(":", 1)[0].strip() + ".p")
        elif key == "Failure":
            entry[key] = failuremap[clean_key(value, failuremap)]
        elif value != "":
            entry[key] = value
    return entry

def process_file(data, features, archivename, path, fileopener, info):
    if verbose:
        print("Processing: ", path)
    problemname   = basename(dirname(path))
    configname    = "_".join(basename(dirname(dirname(path))).split("___")[1:])
    mo            = version_re.search(basename(dirname(dirname(path))).split("_", 1)[0])
    if mo:
        eversion = mo.group()
    else:
        eversion = "unknownVersion"
    fileextension = splitext(path)[-1]
    filename      = basename(path)
    if problemname and configname and fileextension == ".txt" \
       and (("+" in problemname) or ("-" in problemname or ("_" in problemname) or ("^" in problemname))):
        entry = make_entry(fileopener(info).readlines())
        if entry:
           if "Problem" not in entry:
               entry["Problem"] = problemname
           entry.update({"Configname":configname,
                         "Filename":filename,
                         "Archivename":archivename})
           if int(entry.get("Proof object given clauses", 0)) > \
              int(entry.get("Proof search given clauses", 0)):
              #fix output error in e version 1.9.1pre005
              swap(entry, "Proof object given clauses", "Proof search given clauses")
           if "Status" not in entry:
               entry["Status"] = statusmap["unknown"]
               if "Failure" not in entry:
                   entry["Failure"] = failuremap["unknown"]
           if "Failure" not in entry:
               entry["Failure"] = "success"
           if "Version" not in entry:
               entry["Version"] = eversion
           if entry["Failure"] == failuremap["exec failed"]:
               entry["Problem"] = problemname
               entry["Version"] = eversion
           if "Problem" in entry and entry["Problem"] in features:
               entry.update(features[entry["Problem"]])
           if not configname in data or \
              not problemname in data[configname] \
              or entry["Failure"]!=failuremap["unknown"]:
               data[configname][problemname] = entry

def swap(d,key1,key2):
    d[key1],d[key2] = d[key2],d[key1]

def parse_args():
    parser = argparse.ArgumentParser(description="Parse Starexec Cluster Job Output")
    parser.add_argument("infile", nargs="*",  help="input tar or zip files or directory structure")
    parser.add_argument("--header", help="add csv header", action="store_true")
    parser.add_argument("--default", help="default value", default="-")
    parser.add_argument("--delimiter", help="csv delimiter", default=" ")
    parser.add_argument("--metadata", help="add information parsed from file paths", action="store_true")
    parser.add_argument("--compact", help="do not add alignment whitespace", action="store_true")
    parser.add_argument("--features", help="add feature columns with features of the problem")
    parser.add_argument("--verbose", help="be verbose", action="store_true")
    return parser.parse_args()

def read_features(path):
    features = defaultdict(dict)
    if isfile(args.features):
        with open(args.features,"r") as featurefile:
            for line in featurefile.readlines():
                name, _, ptype = line.split(":")
                name = name.strip()
                ptype = ptype.strip()
                if  ptype[0] == "H" or ptype[:2] == "UH":
                    features[name]["Type"] = "horn"
                elif ptype[:2] == "UU":
                    features[name]["Type"] = "unit"
                else:
                    features[name]["Type"] = "general"
                if ptype[2] == "S" or ptype[2] == "P":
                    features[name]["Equational"] = "equational"
                else:
                    features[name]["Equational"] = "non-equational"
    else:
        print("Could not open feature file {}".format(args.features))
        sys.exit(1)
    return features

if __name__ == "__main__":
    global verbose

    verbose = False

    data = defaultdict(dict)
    args = parse_args()
    if args.verbose:
        verbose = True

    features = read_features(args.features) if args.features else defaultdict(dict)

    for infile in args.infile:
        print("processing %s" % infile)
        if isdir(infile):
            for root, _, files in os.walk(infile):
                for filename in files:
                    path = os.path.join(root, filename)
                    process_file(data, features,
                                 basename(dirname(infile)),
                                 path,
                                 lambda p: open(p, mode="rb"), path)
        elif tarfile.is_tarfile(infile):
            with tarfile.open(infile) as tfile:
                for tinfo in tfile:
                    process_file(data, features, basename(infile),
                                 tinfo.name, tfile.extractfile, tinfo)
        elif zipfile.is_zipfile(infile):
            with zipfile.ZipFile(infile) as zfile:
                for zinfo in zfile.infolist():
                    process_file(data, features,
                                 basename(infile),
                                 zinfo.filename,
                                 zfile.open, zinfo)
        else:
            print("Do not know how to open {}.".format(infile))

    keys = set(chain.from_iterable(entry.keys()
                                   for problems in data.values()
                                   for entry in problems.values()))
    fieldnames = firstkeys + sorted(keys.difference(set(firstkeys +
                                                        removekeys +
                                                        metakeys +
                                                        featurekeys)))
    if args.metadata:
        fieldnames += metakeys
    if args.features:
        fieldnames += featurekeys
    for configname, problems in data.items():
        with open(protfile(configname), "w") as report:
            try:
                report.write("# {0[Command]} \n".format(firstvalue(problems)))
            except KeyError:
                report.write("# Could not find command\n")
            report.writelines("# {:>2} {} \n".format(*pair)
                              for pair in enumerate(fieldnames, 1))
            if not args.header:
                report.write("#")
            report.write(args.delimiter.join(fieldnames)+"\n")
            for entrykey in sorted(problems.keys()):
                if args.compact:
                    values = [problems[entrykey].get(key,
                                   args.default)
                              for key in fieldnames]
                else:
                    values = [adjustmap[key](problems[entrykey].get(key, args.default))
                              for key in fieldnames]
                report.write(args.delimiter.join(values)+"\n")
