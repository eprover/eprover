#!/usr/bin/env python3.10
# ----------------------------------
#
# examine_intmap.py
#
# Run E with available test-problems to examine usage and density
# of the current intMap-datatype.
#
# Version history:
# 0.1 Wed Oct  11 19:33:30 CEST 2023
#     Draft version
# 0.2 Wed Jan  10 00:00:30 CEST 2024
#     Productive version

"""
examine_intmap.py

Usage: examine_intmap.py [Options]

Read a single or multiple directories, select problem-files as input for E.
Filters and uses the output to create CSV-Files.

Options:

-h
 Print this information and exit.

<url_1> <url_2>
url_1 -> problem path; instead of a link 'default' can be used
url_2 -> target path; instead of a link 'default' can be used

<url_1> <url_2> <mode>
url_1 -> problem path; instead of a link 'default' can be used
url_2 -> target path; instead of a link 'default' can be used
mode -> mode, optional; default is multiple.
(Other Options include: s(sinlge), f(from file)

?Copyright 2023 ???Denis Feuerstein, schulz@eprover.org???

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

Dr. Stephan Schulz
Hirschstr. 35
76144 Karlsruhe
Germany

or via email (address above).
"""


####################################
# IMPORTS
####################################
import re
import os
import sys
from subprocess import Popen, PIPE


####################################
# SET GLOBAL VARIABLES
####################################
# HELP-TEXT
help_text = "examine_intmap.py\n\n"
help_text = help_text + "Usage: examine_intmap.py [Options]\n\n"
help_text = help_text + "Read a single or multiple directories, "
help_text = help_text + "select problem-files as input for E."
help_text = help_text + "Filters and uses the output to create CSV-Files.\n\n"
help_text = help_text + "Options:\n\n"
help_text = help_text + "-h\nPrint this information and exit.\n\n"
help_text = help_text + "<url_1> <url_2>\n"
help_text = help_text + "url_1 -> problem path; instead of a "
help_text = help_text + "link 'default' can be used\n"
help_text = help_text + "url_2 -> target path; instead of a "
help_text = help_text + "link 'default' can be used\n\n"
help_text = help_text + "<url_1> <url_2> <mode>\n"
help_text = help_text + "url_1 -> problem path; instead of a link"
help_text = help_text + " 'default' can be used\n"
help_text = help_text + "url_2 -> target path; instead of a link"
help_text = help_text + " 'default' can be used\n"
help_text = help_text + "mode -> mode, optional; default is multiple\n"

# ARGUMENT-VARIABLES
eprover_path = './../PROVER/eprover'
problem_path = '../EXAMPLE_PROBLEMS'
target_path = '../../'
mode = "m"  # multi or single directory mode
# ./eprover --auto --memory-limit=2000
# ../../TPTP-v8.2.0/Problems/COL/COL002-1.p
# ../../TPTP-v8.2.0/Problems/HWC

statement_lst = []
directories_lst = []
problem_lst = []
lines = {}
csv_lines = {}

keyword_common_stats_lst = [
 "Status",
 "Reason",
# "ParsedAxioms",
# "RemovedByRelevancyPruning",
# "InitialClauses",
# "RemovedClausePreprocessing",
# "InitialClausesInSaturation",
# "ProcessedClauses",
# "ProcClausTrivial",
# "ProcClausSubsumed",
# "ProcClausRemaining",
 "EmptyInvoked",
 "IntInvoked",
 "ArrayInvoked",
 "TreeInvoked",
 "MapDeleted",
 "EmptyDeleted",
 "IntDeleted",
 "ArrayDeleted",
 "TreeDeleted",
 "IntmapItems",
 "UserTime",
 "SystemTime",
 "TotalTime"]
common_stats_dct = {}

# CONDITIONS FOR COMMON STATS
sub_str_status_1 = "# No proof found!"
sub_str_status_2 = "# Proof found!"
sub_str_status_3 = "# Failure: Out of unprocessed clauses!"
sub_str_status_4 = "# Failure: Resource limit exceeded (memory)"
sub_str_status_5 = "# Failure: Resource limit exceeded (time)"
sub_str_status_6 = "# Failure: User resource limit exceeded"
sub_str_parsed_axioms = "# Parsed axioms "
sub_str_removed_pruning = "# Removed by relevancy pruning/SinE"
sub_str_init_clauses = "# Initial clauses  "
sub_str_removed_preproc = "# Removed in clause preprocessing"
sub_str_init_clauses_sat = "# Initial clauses in saturation"
sub_str_proc_clauses = "# Processed clauses"
sub_str_proc_clauses_triv = "# ...of these trivial"
sub_str_proc_clauses_sub = "# ...subsumed"
sub_str_proc_clauses_rem = "# ...remaining for further processing"
sub_str_empty_invoked = "# Empty invoked times"
sub_str_int_invoked = "# Int invoked times"
sub_str_array_invoked = "# Array invoked times"
sub_str_tree_invoked = "# Tree invoked times"
sub_str_map_deleted = "# Map deleted times"
sub_str_empty_deleted = "# Empty deleted times"
sub_str_int_deleted = "# Int deleted times"
sub_str_array_deleted = "# Array deleted times"
sub_str_tree_deleted = "# Tree deleted times"
sub_str_map_items = "# Intmap-Tree items"
sub_str_user_time = "# User time"
sub_str_system_time = "# System time"
sub_str_total_time = "# Total time"

# CONDITIONS FOR SPECIFIC STATS
sub_string_header = "IntMapType;"
#sub_string_IMEmpty = '"IMEmpty";'
#sub_string_IMSingle = '"IMSingle";'
sub_string_IMArray = '"IMArray";'
sub_string_IMTree = '"IMTree";'


####################################
# ARGUMENTS
####################################
arr_len = len(sys.argv)

if(arr_len == 2):
    # print help
    if(sys.argv[1] == '-h'):
        print(help_text)
        exit()
    else:
        print("Invalid Statement")
elif (2 < arr_len and arr_len < 5):
    # setting paths
    if(sys.argv[1] == 'default'):
        target_path = sys.argv[2] + '/'
    elif(sys.argv[2] == 'default'):
        if(sys.argv[1][-1] == '/'):
            problem_path = sys.argv[1][:-1]
        else:
            problem_path = sys.argv[1]
    elif(sys.argv[1] != 'default' and sys.argv[2] != 'default'):
        if(sys.argv[1][-1] == '/'):
            problem_path = sys.argv[1][:-1]
        else:
            problem_path = sys.argv[1]
        target_path = sys.argv[2] + '/'

    # setting mode
    if(arr_len == 4):
        if(sys.argv[3] == "s" or sys.argv[3] == "f"):
            mode = sys.argv[3]
        else:
            print("Using default setting.")
else:
    print("Too many/few arguments! Going on with default setting.")

print(sys.argv)


####################################
####################################
###### FUNCTION DEFINITION #########
####################################
####################################

####################################
# GET FILES AND DIRECTORIES
####################################
def select_directories(problem_path):
    """Select sub-directories according to given problem-path

    Args:
        problem_path: The given path to the problems
    Returns:
        List
    """

    # list files and directories
    example_directories = os.listdir(problem_path)

    # remove every list-item, that is not a directory
    for directory in example_directories:
        if(not(os.path.isdir(problem_path + '/' + directory))):
            example_directories.remove(directory)

    return example_directories


def return_directories(problem_path, mode):
    """Returns either a list of directories or the given
    problem_path-String as list

    Args:
        problem_path: The given path to the problems
        mode: controls if directory-list or problem_path-String as list
    Returns:
        List
    """

    if(mode == 'm'):
        return select_directories(problem_path)
    elif(mode == 's' or mode == 'f'):
        return [problem_path]


def select_files(problem_path, directories_lst):
    """Select problem-files according to given path and build list

    Args:
        problem_path: Name of the path to the problems
        directories_lst: Sub-directories to the given problem_path
    Returns:
        List
    """

    problem_lst = []

    # Select directory-items, find out if they are files or not and add them to the list
    for directory in directories_lst:
        files_lst = []
        files_lst.extend(os.listdir(problem_path + '/' + directory))

        for problem_file in files_lst:
            if(os.path.isfile(problem_path + '/' + directory + '/' + problem_file)
             and ('.p' in problem_file or '.tptp' in problem_file )):
                # '.lop' in problem_file or
                # print(problem_path + '/' + directory + '/' + problem_file)
                problem_lst.append(problem_path + '/' + directory + '/' + problem_file)

    return problem_lst


def search_problem_files(problem_path, file_path):
    """Returns a list of the hard problems from file
    with hard problems.

    Args:
        file_path: Path to the file with hard problems
        problem_path: The given path to the problems
    Returns:
        List
    """

    p_lst = []
    with open(file_path, 'r') as file:
        lines = file.readlines()
        for line in lines:
            if problem_path in line:
                p_lst.append(line.strip())
    return p_lst


def return_files(problem_path, directories_lst, mode):
    """Select problem-files according to given path and mode

    Args:
        problem_path: Name of the path to the problems
        directories_lst: Sub-directories to the given problem_path
        mode: controls if only one directory or multiple
        directories are scanned or if taken from a file
    Returns:
        List
    """

    if(mode == 'm'):
        return select_files(problem_path, directories_lst)
    elif(mode == 's'):
        problem_lst = []
        files_lst = []
        files_lst.extend(os.listdir(problem_path))
        for problem_file in files_lst:
            if(os.path.isfile(problem_path + '/' + problem_file)
             and ('.p' in problem_file or '.tptp' in problem_file)):
                problem_lst.append(problem_path + '/' + problem_file)
        return problem_lst
    elif(mode == 'f'):
        return search_problem_files(problem_path, r'hard_problems_list.txt')


####################################
# BUILD EPROVER CALL
####################################
def select_options():
    """Returns the options for the eprover. In future implementations
    will dynamically return the parameters according to input

    Args:
        None
    Returns:
        List
    """

    eprover_options = ['--print-statistics', '--auto', '--detsort-rw', '--detsort-new', '-R', '--soft-cpu-limit=5', '--cpu-limit=10', '--memory-limit=2048']
    return eprover_options


# function to build list -> return(statement_lst)
def build_list(eprover_path, eprover_options, problem_path):
    statement = []

    statement.append(eprover_path)
    statement.extend(eprover_options)
    statement.append(problem_path)

    statement_lst.append(statement)
    return statement_lst
#####


####################################
# STRING EXTRACT FUNCTIONS
####################################
def string_select(input_string):
    """ Select the string according to searched pattern

    Args:
        input_string: Given string from standard-input
    Returns:
        Boolean
    """

    if(sub_string_header in input_string):
        return True
    # elif(sub_string_IMEmpty in input_string):
        # return True
    # elif(sub_string_IMSingle in input_string):
        # return True
    elif(sub_string_IMArray in input_string):
        return True
    elif(sub_string_IMTree in input_string):
        return True
    else:
        return False


def select_csv_lines(input_string):
    """ Select the string according to searched pattern

    Args:
        input_string: incomming string
    Returns:
        Boolean
    """

    if(sub_str_status_1 in input_string):
        return True
    elif(sub_str_status_2 in input_string):
        return True
    elif(sub_str_status_3 in input_string):
        return True
    elif(sub_str_status_4 in input_string):
        return True
    elif(sub_str_status_5 in input_string):
        return True
    elif(sub_str_status_6 in input_string):
        return True
    # elif(sub_str_parsed_axioms in input_string):
        # return True
    # elif(sub_str_removed_pruning in input_string):
        # return True
   # elif(sub_str_init_clauses in input_string):
        # return True
    # elif(sub_str_removed_preproc in input_string):
        # return True
    # elif(sub_str_init_clauses_sat in input_string):
        # return True
    # elif(sub_str_proc_clauses in input_string):
        # return True
    # elif(sub_str_proc_clauses_triv in input_string):
        # return True
    # elif(sub_str_proc_clauses_sub in input_string):
        # return True
    # elif(sub_str_proc_clauses_rem in input_string):
        # return True
    elif(sub_str_empty_invoked in input_string):
        return True
    elif(sub_str_int_invoked in input_string):
        return True
    elif(sub_str_array_invoked in input_string):
        return True
    elif(sub_str_tree_invoked in input_string):
        return True
    elif(sub_str_map_deleted in input_string):
        return True
    elif(sub_str_empty_deleted in input_string):
        return True
    elif(sub_str_int_deleted in input_string):
        return True
    elif(sub_str_array_deleted in input_string):
        return True
    elif(sub_str_tree_deleted in input_string):
        return True
    elif(sub_str_map_items in input_string):
        return True
    elif(sub_str_user_time in input_string):
        return True
    elif(sub_str_system_time in input_string):
        return True
    elif(sub_str_total_time in input_string):
        return True
    else:
        return False


def extract_values_into_dict(common_stats_line_dct, stats):
    """Filters the incomming string, extracts the value
    and puts it in the corresponding cell of the dictionary

    Args:
        common_stats_line_dct: dictionary, later used to build CSV-line
        stats: incomming string
    Returns:
        None
    """

    if(sub_str_status_1 in stats):
        common_stats_line_dct["Status"] = "N"
        common_stats_line_dct["Reason"] = "success"
    elif(sub_str_status_2 in stats):
        common_stats_line_dct["Status"] = "T"
        common_stats_line_dct["Reason"] = "success"
    elif(sub_str_status_3 in stats):
        common_stats_line_dct["Status"] = "F"
        common_stats_line_dct["Reason"] = "incomplete"
    elif(sub_str_status_4 in stats):
        common_stats_line_dct["Status"] = "U"
        common_stats_line_dct["Reason"] = "maxmem"
    elif(sub_str_status_5 in stats):
        common_stats_line_dct["Status"] = "U"
        common_stats_line_dct["Reason"] = "maxtime"
    elif(sub_str_status_6 in stats):
        common_stats_line_dct["Status"] = "U"
        common_stats_line_dct["Reason"] = "maxres"
    # elif(sub_str_parsed_axioms in stats):
        # common_stats_line_dct["ParsedAxioms"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_removed_pruning in stats):
        # common_stats_line_dct["RemovedByRelevancyPruning"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_init_clauses in stats):
        # common_stats_line_dct["InitialClauses"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_removed_preproc in stats):
        # common_stats_line_dct["RemovedClausePreprocessing"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_init_clauses_sat in stats):
        # common_stats_line_dct["InitialClausesInSaturation"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_proc_clauses in stats):
        # common_stats_line_dct["ProcessedClauses"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_proc_clauses_triv in stats):
        # common_stats_line_dct["ProcClausTrivial"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_proc_clauses_sub in stats):
        # common_stats_line_dct["ProcClausSubsumed"] = stats[stats.rfind(': ')+2:]
    # elif(sub_str_proc_clauses_rem in stats):
        # common_stats_line_dct["ProcClausRemaining"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_empty_invoked in stats):
        common_stats_line_dct["EmptyInvoked"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_int_invoked in stats):
        common_stats_line_dct["IntInvoked"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_array_invoked in stats):
        common_stats_line_dct["ArrayInvoked"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_tree_invoked in stats):
        common_stats_line_dct["TreeInvoked"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_map_deleted in stats):
        common_stats_line_dct["MapDeleted"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_empty_deleted in stats):
        common_stats_line_dct["EmptyDeleted"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_int_deleted in stats):
        common_stats_line_dct["IntDeleted"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_array_deleted in stats):
        common_stats_line_dct["ArrayDeleted"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_tree_deleted in stats):
        common_stats_line_dct["TreeDeleted"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_map_items in stats):
        common_stats_line_dct["IntmapItems"] = stats[stats.rfind(': ')+2:]
    elif(sub_str_user_time in stats):
        common_stats_line_dct["UserTime"] = re.sub(" s", "", stats[stats.rfind(': ')+2:])
    elif(sub_str_system_time in stats):
        common_stats_line_dct["SystemTime"] = re.sub(" s", "", stats[stats.rfind(': ')+2:])
    elif(sub_str_total_time in stats):
        common_stats_line_dct["TotalTime"] = re.sub(" s", "", stats[stats.rfind(': ')+2:])


####################################
# STRING MANIPULATION FUNCTIONS
####################################
def string_clean(line_str):
    """gets a line as a string and cleans it from unwanted bi-products

    Args:
        line_str: String to clean
    Returns:
        String
    """

    return re.sub("b'|'|\\\\n", "", line_str)


def return_problem_name(problem):
    """Returns problem-name

    Args:
        problem: problem-path with problem-filename
    Returns:
        String
    """

    return re.sub("|\.p|\.lop|\.tptp", "", problem[problem.rfind('/')+1:])


def return_file_name(target_path, problem):
    """Returns file-path with problem-filename

    Args:
        target_path: path to directory
        problem: problem-path with problem-filename
    Returns:
        String
    """

    if (target_path == "../../"):
        file_name = target_path + return_problem_name(problem) + ".csv"
        return file_name
    else:
        if(os.path.isdir(target_path)):
            file_name = target_path + return_problem_name(problem) + ".csv"
            return file_name
        else:
            print("Directory not found - using default directory")
            file_name = "../../" + return_problem_name(problem) + ".csv"
            return file_name


def get_problem_directory(problem_path, mode):
    """Returns the problem-directory without path and with underscore

    Args:
        problem_path: path to problem-directory
        mode: single or multiple
    Returns:
        String
    """

    if(mode == 's' or mode == 'f'):
        #problem_path[problem_path[:-1].rfind('/')+1:-1])
        return (problem_path[problem_path[:-1].rfind('/')+1:] + "_")
    else:
        return ""


def get_target_path(target_path, mode):
    """Returns the target-path and according to mode with added
    directory - Attention: only worrks if the directory exists!

    Args:
        target_path: path to target-directory
        mode: single or multiple
    Returns:
        String
    """

    if(os.path.exists(target_path + "/common_stats") and (mode == 's' or mode == 'f')):
        return target_path + "common_stats/"
    else:
        return target_path


####################################
# CONVERT INTO CSV FUNCTIONS
####################################
def build_header(keyword_common_stats_lst):
    """Builds the header for the CSV-file based on input-list

    Args:
        keyword_common_stats_lst: list of keywords
    Returns:
        String
    """
    header_str = "Problem"
    for keyword in keyword_common_stats_lst:
        header_str = header_str + ";" + keyword

    return header_str


def convert_dict_into_str(prob_str, common_stats_line_dct):
    """Converts Dictionary into a String to be used as a line
    for the CSV-File

    Args:
        prob_str: name of the processed problem
        common_stats_line_dct: dictionary with collected data
    Returns:
        String
    """
    csv_str = prob_str
    for stat in common_stats_line_dct:
        csv_str = csv_str + ";" + common_stats_line_dct[stat]

    return csv_str



####################################
# EXECUTION FUNCTIONS
####################################
def execute_statement(statement_lst):
    """Executes the statement, described by the arguments
    given in the list, since Popen() awaits a list of arguments

    Args:
        statement_lst: list of arguments
    Returns:
        List
    """

    str_not_empty = True
    lines = []
    csv_lines = []

    #Open subprocess with list of arguments and pipe it
    process = Popen(statement_lst, shell=False, stdout=PIPE, stdin=PIPE)

    #Build list with output from subprocess
    while str_not_empty:
        line = process.stdout.readline()

        if(line == b''):
            str_not_empty = False
        else:
            if(string_select(str(line))):
                lines.append(string_clean(str(line)))
            elif(select_csv_lines(str(line))):
                csv_lines.append(string_clean(str(line)))

    process.kill()

    return [lines, csv_lines]


####################################
####################################
####### EXECUTION DIVISION #########
####################################
####################################

####################################
# SET VARIABLES AND OPTIONS
####################################

directories_lst = return_directories(problem_path, mode)
problem_lst = return_files(problem_path, directories_lst, mode)
eprover_options = select_options()


####################################
# CALL E AND SCAN OUTPUT
####################################

for problem in problem_lst:
    if(problem_path != "../EXAMPLE_PROBLEMS/SMOKETEST/tffex01.p"):
        print("problem: \t" + problem)
        statement_lst = []
        statement_lst.append(eprover_path)
        statement_lst.extend(eprover_options)
        statement_lst.append(problem)
        temp_lines = execute_statement(statement_lst)
        lines[problem] = temp_lines[0]
        csv_lines[problem] = temp_lines[1]


####################################
# BUILD CSV-FILES
####################################

for p in problem_lst:
    if(problem_path != "../EXAMPLE_PROBLEMS/SMOKETEST/tffex01.p"):
        file_name = return_file_name(target_path, p)

        common_stats_dct[p] = dict.fromkeys(keyword_common_stats_lst, " ")
        for stat_line in csv_lines[p]:
            extract_values_into_dict(common_stats_dct[p], stat_line)

        with open(file_name, 'w') as out_file:
            out_file.write('{0}\n'.format('\n '.join(str(n) for n in lines[p])))


####################################
# BUILD COMMON-CSV-FILE
####################################

csv_input_lst = [build_header(keyword_common_stats_lst)]

for p in problem_lst:
    if(problem_path != "../EXAMPLE_PROBLEMS/SMOKETEST/tffex01.p"):
        csv_input_lst.append(convert_dict_into_str(return_problem_name(p), common_stats_dct[p]))

with open(get_target_path(target_path, mode) + get_problem_directory(problem_path, mode) + "common_stats.csv", 'w') as out_file:
    out_file.write('{0}\n'.format('\n '.join(str(n) for n in csv_input_lst)))


####################################
# END OF FILE
####################################
