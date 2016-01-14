#!/usr/bin/env python3

import argparse
import tarfile
import zipfile
import csv

from itertools import chain
from os.path import dirname, splitext, basename
from collections import defaultdict


firstkeys  = ["Problem", "Status", "User time", "Failure", "Version", "Preprocessing time"]
removekeys = ["eprover","Command", "Computer", "Model", "CPU", "Memory", "OS", "CPULimit", "DateTime", "CPUTime"]


failuremap = {"User resource limit exceeded"    :"maxres    ",
              "Out of unprocessed clauses!"     :"incomplete",
              "Resource limit exceeded (memory)":"maxmem    ",
              "Resource limit exceeded (time)"  :"maxtime   "}


statusmap = {"SZS status GaveUp"            :"F",
			 "SZS status ResourceOut"       :"F",
			 "SZS status Satisfiable"       :"N",
			 "SZS status CounterSatisfiable":"N",
			 "SZS status Theorem"           :"T",
			 "SZS status Unsatisfiable"     :"T"}


def isCNForFOF(path):
	problemname = basename(dirname(path))
	return splitext(path)[-1] == ".txt" and (("+" in problemname) or ("-" in problemname))


def cleanvalue(value):
	value = value.strip()
	if value.endswith(" s"):
		value = value[:-2]
	elif value.endswith(" pages"):
		value = value[:-6]
	elif value.endswith("MB"):
		value = value[:-2]
	return value


def removetimestamp(line):
	# split prefixed timestamp X.XX/X.XX % or X.XX/X.XX #
	split = line.split("%", 1)
	if len(split) == 2:
		return split[1].strip()
	else:
		split = line.split("#", 1)
		if len(split) == 2:
			return split[1].strip()
		else:
			# line with only timestamp
			return ""

def makeentry(lines):
	entry = {"Failure":"success   "}
	for line in lines:
		line = line.decode()
		line = removetimestamp(line)
		split = line.split(":", 1)
		key   = split[0].strip()
		value = cleanvalue(split[1]) if len(split) == 2 else ""
		if key.startswith("SZS status"):
			entry["Status"] = statusmap[key]
		elif key == "Problem":
			entry[key] = (value.split(":", 1)[0].strip() + ".p").ljust(12)
		elif key == "Failure":
			entry[key] = failuremap[value]
		elif key == "User time":
			entry[key] = value.rjust(8)
		elif key == "Preprocessing time":
			entry[key] = value.rjust(8)
		elif value != "":
			entry[key] = value.rjust(12)
	return entry

def heuristicname(path):
	return "_".join(dirname(dirname(path)).split("_")[-2:])

if __name__ == "__main__":

	parser = argparse.ArgumentParser(description="Parse Starexec Cluster Job Output")
	parser.add_argument("infiles", nargs="*",  help="input tar or zip files")
	args = parser.parse_args()

	data = defaultdict(dict)
	for infile in args.infiles:
		if tarfile.is_tarfile(infile):
			with tarfile.open(infile) as tfile:
				for tinfo in tfile:
					path = tinfo.name
					if tinfo.isfile() and isCNForFOF(path):
						#print(infile+" : "+path)
						data[heuristicname(path)][path] = makeentry(tfile.extractfile(path).readlines())
		elif zipfile.is_zipfile(infile):
			with zipfile.ZipFile(infile) as zfile:
				for zinfo in zfile.infolist():
					path = zinfo.filename
					if isCNForFOF(path):
						#print(infile+" : "+path)
						data[heuristicname(path)][path] = makeentry(zfile.open(path).readlines())
		else:
			print("Dont know how to open %s." % infile)

	keys       = set(chain.from_iterable(entry.keys() for problems in data.values() for entry in problems.values()))
	fieldnames = firstkeys + sorted(keys.difference(set(firstkeys + removekeys)))

	for heuristic, problems in data.items():
		with open("protokoll_G----_" + heuristic + ".csv", "w", newline="") as report:
			report.write("#" + next(iter(problems.values()))["Command"] + "\n")
			report.writelines(("#" + str(number) + " " + name + "\n" for number, name in enumerate(fieldnames, 1)))
			reportwriter = csv.DictWriter(report, restval="-".rjust(8), extrasaction="ignore", fieldnames=fieldnames)
			report.write("#")
			reportwriter.writeheader()
			for entrykey in sorted(problems.keys()):
				reportwriter.writerow(problems[entrykey])
