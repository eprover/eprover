#!/usr/bin/env python2.7
# ----------------------------------
#
# examine_intmap.py
#
# Run E with available test-problems to examine usage and density
# of the current intMap-datatype.
#
# Version history:
# 0.1 Wed Oct  11 21:10:30 CEST 2006
#     Draft version

"""
examine_intmap.py

Usage: examine_intmap.py [Options]

Read an input file, determine the problem class, select a suitable
schedule and run E accordingly.

?Restriction: The initial version only reads a fixed format and does
?not support extra options.

Options:

-h
 Print this information and exit.

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

#function to select directories                     X
#function to select problem-files                   X
#function to select options                         X
#function to build list -> return(statement_lst)    X
#function to run the statement                      X
#function to filter the returned lines
#function to clean string
#function to build csv file
#function to throw out documentation-string




###########
# Imports #
###########

import re
import os
from subprocess import Popen, PIPE




#############
# Variables #
#############

eprover_path = './../PROVER/eprover'
problem_path = '../EXAMPLE_PROBLEMS'
statement_lst = []
directories_lst = []
problem_lst = []
lines = {}



#############
# Functions #
#############

def select_directories(problem_path):
    """Select sub-directories according to given problem-path

    Args:
        problem_path: Name of the given path
    Returns:
        List
    """

    #list files and directories
    example_directories = os.listdir(problem_path)

    #remove every list-item, that is not a directory
    for directory in example_directories:
        if(not(os.path.isdir(problem_path + '/' + directory))):
            example_directories.remove(directory)

    return example_directories




def select_files(problem_path, directories_lst):
    """Select problem-files according to given path and build list

    Args:
        problem_path: Name of the path to the problems
        directories_lst: Sub-directories to the given problem_path
    Returns:
        List
    """

    problem_lst = []

    #Select directory-items, find out if they are files or not and add them to the list
    for directory in directories_lst:
        files_lst = []
        files_lst.extend(os.listdir(problem_path + '/' + directory))

        for problem_file in files_lst:
            if(os.path.isfile(problem_path + '/' + directory + '/' + problem_file) and ('.lop' in problem_file or  '.p' in problem_file or '.tptp' in problem_file )):
               problem_lst.append(problem_path + '/' + directory + '/' + problem_file)

    return problem_lst




def select_options ():
    """Select options ...

    Args:
        ...
    Returns:
        List
    """

    eprover_options = ['--print-statistics', '--auto', '--memory-limit=2000']
    return eprover_options




####function to build list -> return(statement_lst)
def build_list(eprover_path, eprover_options, problem_path):
    statement = []

    statement.append(eprover_path)
    statement.extend(eprover_options)
    statement.append(problem_path)

    statement_lst.append(statement)
    return statement_lst
#####



def string_select(input_string):
    """xxxxxxxxxxxxxxx

    Args:
        input_string: xxx
    Returns:
        Boolean
    """


    sub_string_no_proof = "# No proof found!"
    sub_string_found_proof = "# Proof found!"
    sub_string_total_time = "# Total time"
    sub_string_inv_int = '# Int invoked times'
    sub_string_inv_arr = '# Array invoked times'
    sub_string_inv_tree = '# Tree invoked times'
    sub_string_deleted = '# Map deleted times'

    sub_string_type_array = '# IntMap-Type: \\t IMArray'
    sub_string_getval_array = '# Executed function IntMapGetVal (IMArray '
    sub_string_getref_array = '# Executed function IntMapGetRef (IMArray '
    sub_string_assign_array = '# Executed function IntMapAssign (IMArray '
    sub_string_delkey_array = '# Executed function IntMapDelKey (IMArray '

    sub_string_type_tree = '# IntMap-Type: \\t IMTree'
    sub_string_getval_tree = '# Executed function IntMapGetVal (IMTree '
    sub_string_getref_tree = '# Executed function IntMapGetRef (IMTree '
    sub_string_assign_tree = '# Executed function IntMapAssign (IMTree '
    sub_string_delkey_tree = '# Executed function IntMapDelKey (IMTree '
    sub_string_nodes_tree = '# Nodes inside of Map (IMTree'

    if(sub_string_no_proof in input_string):
        return True
    elif(sub_string_found_proof in input_string):
        return True
    elif(sub_string_total_time in input_string):
        return True
    elif(sub_string_inv_int in input_string):
        return True
    elif(sub_string_inv_arr in input_string):
        return True
    elif(sub_string_inv_tree in input_string):
        return True
    elif(sub_string_deleted in input_string):
        return True
    elif(sub_string_type_array in input_string):
        return True
    elif(sub_string_getval_array in input_string):
        return True
    elif(sub_string_getref_array in input_string):
        return True
    elif(sub_string_assign_array in input_string):
        return True
    elif(sub_string_delkey_array in input_string):
        return True
    elif(sub_string_type_tree in input_string):
        return True
    elif(sub_string_getval_tree in input_string):
        return True
    elif(sub_string_getref_tree in input_string):
        return True
    elif(sub_string_assign_tree in input_string):
        return True
    elif(sub_string_delkey_tree in input_string):
        return True
    elif(sub_string_nodes_tree in input_string):
        return True
    else:
        return False




def string_clean():
    """gets a list,  returns a data frame

    Args:
        input_string: xxx
    Returns:
        Boolean
    """
    #re.find(r'\d', string)
    pass




def execute_statement(statement_lst):
    """Executes the statement, described by the arguments given in the list, since Popen() awaits a list of arguments

    Args:
        statement_lst: list of arguments
    Returns:
        List
    """

    str_not_empty = True
    lines = []

    #Open subprocess with list of arguments and pipe it
    process = Popen(statement_lst, shell=False, stdout=PIPE, stdin=PIPE)

    #Build list with output from subprocess
    while str_not_empty:
        line = process.stdout.readline()

        if(line == b''):
            str_not_empty = False
        else:
            if(string_select(str(line))):
                lines.append(str(line))

    process.kill()

    return lines



###############
# Run Program #
###############

directories_lst = select_directories(problem_path)
problem_lst = select_files(problem_path, directories_lst)
eprover_options = select_options()
#problem_path = '../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p'


for problem in problem_lst:
    if(problem == '../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p' or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p'):
        #statement_lst = build_list(eprover_path, eprover_options, problem)
        statement_lst = []
        statement_lst.append(eprover_path)
        statement_lst.extend(eprover_options)
        statement_lst.append(problem)
        lines[problem] = execute_statement(statement_lst)





print('../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p')
line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p']
print(*line, sep='\n')
"""
print(next((s for s in line if sub_string_inv_int in s), None))
print(next((s for s in line if sub_string_inv_arr in s), None))
print(next((s for s in line if sub_string_inv_tree in s), None))
print(next((s for s in line if sub_string_deleted in s), None))
"""

print('../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p')
line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p']
print(*line, sep='\n')
"""
print(next((s for s in line if sub_string_inv_int in s), None))
print(next((s for s in line if sub_string_inv_arr in s), None))
print(next((s for s in line if sub_string_inv_tree in s), None))
print(next((s for s in line if sub_string_deleted in s), None))
"""

"""
In Operator ist schneller als RegEx
"""


"""
if __name__ == '__main__':
    for option in get_options():
        if option == "-h":
            print __doc__
            sys.exit()
        else:
            sys.exit("Unknown option "+ option)
"""

#Search all files
"""
if(os.name == 'nt'):
    pass # Windows
elif(os.name == 'posix'):
    pass # Linux

for b_file in exec_files:
    print(b_file)
    os.execl( b_file, ' -e ')
    #os.system('.' + b_file)
"""
"""
function process_result(job,    file, tmp,  time, status, reason, name)
{
   time = time_limit;
   status = "F";
   reason = "unknown";
   file = cwd "/__prvout__" procid "_" global_hostname "__";
   name = job;
   processed = 0;
   generated = 0;
   rewrite_steps = 0;
   shared_terms = 0;
   raw_terms = 0;
   r_matches = 0;
   e_matches = 0;
   literals = 0;
   gen_literals = 0;

   while((getline tmp < file)>0)
   {
      if(index(tmp, "# No proof found!"))
      {
	 status = "N";
	 reason = "success";
      }
      else if(index(tmp, "# Proof found!"))
      {
	 status = "T";
	 reason = "success";
      }
      else if(index(tmp, "# Failure: Out of unprocessed clauses!"))
      {
	 status = "F";
	 reason = "incomplete";
      }
      else if(index(tmp, "# Failure: Resource limit exceeded (memory)"))
      {
	 reason = "maxmem ";
      }
      else if(index(tmp, "# Failure: Resource limit exceeded (time)"))
      {
	 reason = "maxtime ";
      }
      else if(index(tmp, "# Failure: User resource limit exceeded"))
      {
	 reason = "maxres";
      }
      else if(index(tmp, "# Processed clauses                    :"))
      {
	 processed = substr(tmp, 42);
      }
      else if(index(tmp, "# Generated clauses                    :"))
      {
	 generated = substr(tmp, 42);
      }
      else if(index(tmp, "# Shared term nodes                    :"))
      {
	 shared_terms = substr(tmp, 42);
      }
      else if(index(tmp, "# ...corresponding unshared nodes      :"))
      {
	 raw_terms = substr(tmp, 42);
      }
      else if(index(tmp, "# ...number of literals in the above   :"))
      {
	 literals = substr(tmp, 42);
      }
      else if(index(tmp, "# Shared rewrite steps                 :"))
      {
	 rewrite_steps = substr(tmp, 42);
      }
      else if(index(tmp, "# Match attempts with oriented units   :"))
      {
	 r_matches = substr(tmp, 42);
      }
      else if(index(tmp, "# Match attempts with unoriented units :"))
      {
	 e_matches = substr(tmp, 42);
      }
      else if(index(tmp, "# Total literals in generated clauses  :"))
      {
	 gen_literals = substr(tmp, 42);
      }
      else if(index(tmp, "# Total time"))
      {
	 time = substr(tmp, 30);
      }
   }
   close(file);
   printf("%-29s " status " %8.3f  %-10s %10d %10d %10d %10d %10d %10d %10d %10d %10d\n", \
	  name, 0+time, reason, generated, processed,shared_terms,raw_terms,\
	  rewrite_steps, r_matches, e_matches, gen_literals,literals) >> logfile;
   printf("%-29s " status " %8.3f  %-10s\n", \
	  name, 0+time, reason);
   system("rm " file);
}

"""
