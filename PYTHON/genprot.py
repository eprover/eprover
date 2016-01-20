#!/usr/bin/env python3

"""
genprot 0.1

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
--metadata   add metadata from directory structure - filename and heuristic name
--strip      strip alignment whitespace

Copyright 2015 Martin Möhrmann, moehrmann@eprover.org

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

The original copyright holder can be contacted as

Martin Möhrmann
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

from itertools import chain
from os.path import dirname, splitext, basename, isdir, isfile
from collections import defaultdict

protfile = (lambda name: "protocol_G----_" + name + ".csv")

firstkeys  = ["Problem", "Status", "User time", "Failure", "Version", "Preprocessing time"]
metakeys   = ["Filename", "Heuristic"]
removekeys = ["eprover","Command", "Computer", "Model", "CPU", "Memory", "OS", "CPULimit", "DateTime", "CPUTime"]


failuremap = {"User resource limit exceeded"    :"maxres      ",
              "Out of unprocessed clauses!"     :"incomplete  ",
              "Resource limit exceeded (memory)":"maxmem      ",
              "Resource limit exceeded (time)"  :"maxtime     ",
              "exec failed"                     :"starexecfail"}


statusmap = {"exec failed"                  :"F",
	         "SZS status GaveUp"            :"F",
			 "SZS status ResourceOut"       :"F",
			 "SZS status Satisfiable"       :"N",
			 "SZS status CounterSatisfiable":"N",
			 "SZS status Theorem"           :"T",
			 "SZS status Unsatisfiable"     :"T"}

adjust = 12

adjustmap     = {"User time"         :8,
                 "Preprocessing time":8,
                 "Status"            :1,
                 "Failure"           :10}

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
	if len(split) == 2:
		return split[1].strip()
	else:
		split = line.split("#", 1)
		if len(split) == 2:
			return split[1].strip()
		else:
			# might be: 0.00/0.00   exec failed: No such file or directory
			split = line.split("\t", 1)
			if len(split) == 2 and split[1].startswith("exec failed"):
				return split[1].strip()
			else:
				# line with only timestamp or other data
				return ""

def make_entry(lines):
	entry = {"Failure":"success   "}
	for line in lines:
		line = line.decode()
		line = remove_timestamp(line)
		split = line.split(":", 1)
		key   = split[0].strip()
		value = clean_value(split[1]) if len(split) == 2 else ""
		if key.startswith("SZS status"):
			entry["Status"] = statusmap[key]
		elif key.startswith("exec failed"):
			entry["Status"] = statusmap["exec failed"]
			entry["Failure"] = failuremap["exec failed"]
		elif key == "Problem":
			entry[key] = (value.split(":", 1)[0].strip() + ".p").ljust(12)
		elif key == "Failure":
			entry[key] = failuremap[value]
		elif value != "":
			entry[key] = value
	return entry

def process_file(data, path, fileopener, info):
	problemname   = basename(dirname(path))
	heuristicname = "_".join(basename(dirname(dirname(path))).split("_")[-2:])
	eversion      = basename(dirname(dirname(path))).split("_",1)[0][2:]
	fileextension = splitext(path)[-1]
	filename      = basename(path)
	if problemname and heuristicname and fileextension == ".txt" and (("+" in problemname) or ("-" in problemname)):
		#print(infile+" : "+path)
		entry = make_entry(fileopener(info).readlines())
		entry.update({"Heuristic":heuristicname.rjust(12),"Filename":filename.rjust(12)})
		#fix output error in e version 1.9.1pre005
		if int(entry.get("Proof object given clauses",0)) > int(entry.get("Proof search given clauses",0)):
			swap(entry,"Proof object given clauses","Proof search given clauses")
		if entry["Failure"] == failuremap["exec failed"]:
			entry["Problem"] = problemname.ljust(12)
			entry["Version"] = eversion
		if not entry.get("Problem","").strip() == problemname:
			print("Error problem directory does not match problemname in file %s" % (filename))
		else:
			data[heuristicname][problemname] = entry

def swap(d,key1,key2):
	d[key1],d[key2] = d[key2],d[key1]

def parse_args():
	parser = argparse.ArgumentParser(description="Parse Starexec Cluster Job Output")
	parser.add_argument("infile", nargs="*",  help="input tar or zip files or directory structure")
	parser.add_argument("--header", help="add csv header", action="store_true")
	parser.add_argument("--default", help="default value", default="-")
	parser.add_argument("--delimiter", help="csv delimiter", default=",")
	parser.add_argument("--metadata", help="add information parsed from file paths", action="store_true")
	parser.add_argument("--strip", help="strip all value", action="store_true")
	return parser.parse_args()

if __name__ == "__main__":

	data = defaultdict(dict)
	args = parse_args()

	for infile in args.infile:
		print("processing %s" % infile)
		if tarfile.is_tarfile(infile):
			with tarfile.open(infile) as tfile:
				for tinfo in tfile:
					process_file(data, tinfo.name, tfile.extractfile, tinfo)
		elif zipfile.is_zipfile(infile):
			with zipfile.ZipFile(infile) as zfile:
				for zinfo in zfile.infolist():
					process_file(data, zinfo.filename, zfile.open, zinfo)
		elif isdir(path):
			for _, _, files in os.walk(path):
				for path in files:
					process_file(data, path, open, path)
		else:
			print("Dont know how to open %s." % infile)

	keys       = set(chain.from_iterable(entry.keys() for problems in data.values() for entry in problems.values()))
	fieldnames = firstkeys + sorted(keys.difference(set(firstkeys + removekeys + metakeys)))
	if args.metadata:
		fieldnames += metakeys
	for heuristic, problems in data.items():
		with open(protfile(heuristic), "w", newline="") as report:
			report.write("#" + next(iter(problems.values()))["Command"] + "\n")
			report.writelines(("#" + str(number) + " " + name + "\n" for number, name in enumerate(fieldnames, 1)))
			if not args.header:
				report.write("#")
			report.write(args.delimiter.join(fieldnames)+"\n")
			for entrykey in sorted(problems.keys()):
				if args.strip:
					values = [problems[entrykey].get(key, args.default).strip() for key in fieldnames]
				else:
					values = [problems[entrykey].get(key, args.default).rjust(adjustmap.get(key, adjust)) for key in fieldnames]
				report.write(args.delimiter.join(values)+"\n")
