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
    """ Select the string according to searched pattern

    Args:
        input_string: Given string from standard-input
    Returns:
        Boolean
    """

    sub_string_immap_stats = "# INTMAP STATS\\n"
    sub_string_imtree_stats = "# STATS-"
    sub_string_imtree_count = "# Intmap-Tree hat"

    sub_string_no_proof = "# No proof found!"
    sub_string_found_proof = "# Proof found!"
    sub_string_total_time = "# Total time"
    sub_string_inv_emp = '# Empty invoked times'
    sub_string_inv_int = '# Int invoked times'
    sub_string_inv_arr = '# Array invoked times'
    sub_string_inv_tree = '# Tree invoked times'
    sub_string_deleted = '# Map deleted times'

    sub_string_type = "# IntMap-Type: \\t "
    sub_string_exec_func = "# Executed function "
    sub_string_map_traits = "# Map-Traits "
    sub_string_values_traits = "# Values-Traits "
    sub_string_number_items = "# Number of items"
    sub_string_value_single = "# Value (IMSingle"

    sub_string_empty_cells = "# Number of empty cells (IMArray"
    sub_string_structure = "# Structure of "
    sub_string_tree_height = "# Tree height (IMTree"


    #sub_string_type_single = "# IntMap-Type: \\t IMSingle";
    #sub_string_type_array = '# IntMap-Type: \\t IMArray'
    #sub_string_getval_array = '# Executed function IntMapGetVal (IMArray '
    #sub_string_getref_array = '# Executed function IntMapGetRef (IMArray '
    #sub_string_assign_array = '# Executed function IntMapAssign (IMArray '
    #sub_string_delkey_array = '# Executed function IntMapDelKey (IMArray '
    #sub_string_nodes_array = '# Nodes inside of Map (IMArray'

    #sub_string_type_tree = '# IntMap-Type: \\t IMTree'
    #sub_string_getval_tree = '# Executed function IntMapGetVal (IMTree '
    #sub_string_getref_tree = '# Executed function IntMapGetRef (IMTree '
    #sub_string_assign_tree = '# Executed function IntMapAssign (IMTree '
    #sub_string_delkey_tree = '# Executed function IntMapDelKey (IMTree '
    #sub_string_nodes_tree = '# Nodes inside of Map (IMTree'

    anzahl = '# Anzahl'

    if(sub_string_immap_stats in input_string):
        return True
    elif(sub_string_imtree_stats in input_string):
        return True
    elif(sub_string_imtree_count in input_string):
        return True
    elif(sub_string_no_proof in input_string):
        return True
    elif(sub_string_found_proof in input_string):
        return True
    elif(sub_string_total_time in input_string):
        return True
    elif(sub_string_inv_emp in input_string):
        return True
    elif(sub_string_inv_int in input_string):
        return True
    elif(sub_string_inv_arr in input_string):
        return True
    elif(sub_string_inv_tree in input_string):
        return True
    elif(sub_string_deleted in input_string):
        return True
    elif(sub_string_type in input_string):
        return True
    elif(sub_string_exec_func in input_string):
        return True
    elif(sub_string_map_traits in input_string):
        return True
    elif(sub_string_values_traits in input_string):
        return True
    elif(sub_string_number_items in input_string):
        return True
    elif(sub_string_value_single in input_string):
        return True
    elif(sub_string_empty_cells in input_string):
        return True
    elif(sub_string_structure in input_string):
        return True
    elif(sub_string_tree_height in input_string):
        return True
    elif(anzahl in input_string):
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
    if(problem == '../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p' or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p'
       or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/BOO020-1.p' or
       #problem == '../EXAMPLE_PROBLEMS/TPTP/BOO006-1.p'
       #problem == '../EXAMPLE_PROBLEMS/TPTP/SET183-6.p'
       #problem == '../EXAMPLE_PROBLEMS/TPTP/SWV851-1.p'
       #or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/GROUP1st.p' or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/LUSK6.lop'
       #or problem == '../EXAMPLE_PROBLEMS/TPTP/BOO010-2.p' or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/LUSK6ext.lop'
       problem == '../EXAMPLE_PROBLEMS/SMOKETEST/ans_test06.p' or problem == '../EXAMPLE_PROBLEMS/SMOKETEST/CNFTest.p'
       ):
        #statement_lst = build_list(eprover_path, eprover_options, problem)
        statement_lst = []
        statement_lst.append(eprover_path)
        statement_lst.extend(eprover_options)
        statement_lst.append(problem)
        lines[problem] = execute_statement(statement_lst)



print('../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p')
line = ['../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p']
line.extend(lines['../EXAMPLE_PROBLEMS/SMOKETEST/ALL_RULES.p'])
print(*line, sep='\n')

print('../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p')
line.extend(['../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p'])
line.extend(lines['../EXAMPLE_PROBLEMS/SMOKETEST/socrates.p'])
print(*line, sep='\n')

print('../EXAMPLE_PROBLEMS/SMOKETEST/ans_test06.p')
line.extend(['../EXAMPLE_PROBLEMS/SMOKETEST/ans_test06.p'])
line.extend(lines['../EXAMPLE_PROBLEMS/SMOKETEST/ans_test06.p'])
print(*line, sep='\n')

print('../EXAMPLE_PROBLEMS/SMOKETEST/BOO020-1.p')
line.extend(['../EXAMPLE_PROBLEMS/SMOKETEST/BOO020-1.p'])
line.extend(lines['../EXAMPLE_PROBLEMS/SMOKETEST/BOO020-1.p'])
print(*line, sep='\n')


print('../EXAMPLE_PROBLEMS/SMOKETEST/CNFTest.p')
line.extend(['../EXAMPLE_PROBLEMS/SMOKETEST/CNFTest.p'])
line.extend(lines['../EXAMPLE_PROBLEMS/SMOKETEST/CNFTest.p'])
print(*line, sep='\n')
"""
print('../EXAMPLE_PROBLEMS/SMOKETEST/ans_test06.p')
line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/ans_test06.p']
print(*line, sep='\n')

print('../EXAMPLE_PROBLEMS/SMOKETEST/CNFTest.p')
line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/CNFTest.p']
print(*line, sep='\n')
"""

#print('../EXAMPLE_PROBLEMS/TPTP/SWV851-1.p')
#line = lines['../EXAMPLE_PROBLEMS/TPTP/SWV851-1.p']
#print('../EXAMPLE_PROBLEMS/TPTP/SET183-6.p')
#line = lines['../EXAMPLE_PROBLEMS/TPTP/SET183-6.p']

#print('../EXAMPLE_PROBLEMS/SMOKETEST/BOO020-1.p')
#line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/BOO020-1.p']
#print(*line, sep='\n')


with open('test.txt', 'w') as out_file:
    out_file.write('{0}\n'.format('\n '.join(str(n) for n in line)))


#print('../EXAMPLE_PROBLEMS/TPTP/BOO006-1.p')
#line = lines['../EXAMPLE_PROBLEMS/TPTP/BOO006-1.p']
#print(*line, sep='\n')

#print('../EXAMPLE_PROBLEMS/SMOKETEST/GROUP1st.p')
#line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/GROUP1st.p']
#print(*line, sep='\n')

#print('../EXAMPLE_PROBLEMS/SMOKETEST/LUSK6.lop')
#line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/LUSK6.lop']
#print(*line, sep='\n')

#print('../EXAMPLE_PROBLEMS/SMOKETEST/LUSK6ext.lop')
#line = lines['../EXAMPLE_PROBLEMS/SMOKETEST/LUSK6ext.lop']
#print(*line, sep='\n')

#print('../EXAMPLE_PROBLEMS/TPTP/BOO010-2.p')
#line = lines['../EXAMPLE_PROBLEMS/TPTP/BOO010-2.p']
#print(*line, sep='\n')





"""
print(next((s for s in line if sub_string_inv_int in s), None))
print(next((s for s in line if sub_string_inv_arr in s), None))
print(next((s for s in line if sub_string_inv_tree in s), None))
print(next((s for s in line if sub_string_deleted in s), None))
"""

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

