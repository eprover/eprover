#!/usr/bin/python3

from e_evaluator import *

import re

FOF_REGEX = r'[A-Z]{3}\d{3}(_|\+|\-]).*\.p$'
FOF_AUTOMATON = re.compile(FOF_REGEX)

def is_fof(file_name):
  return any(FOF_AUTOMATON.finditer(file_name.strip()))

def cmp_binaries(bin1, bin2, test_folder_1, test_folder_2, limit=None, timeout=60):
  report = []

  from os import listdir
  from os.path import isfile, join, basename
  file_names = [f for f in listdir(test_folder_1) if isfile(join(test_folder_1, f))]

  import random as rnd
  rnd.shuffle(file_names)

  if limit==None:
    limit = len(file_names)

  for (i,filepath) in enumerate(file_names[:limit]):
      filepath_1 = join(test_folder_1, filepath)
      filepath_2 = join(test_folder_2, filepath)

      print('Working on {0}/{1}'.format(i+1, limit))
      exp_status = get_theorem_status(filepath_1)
        
      args_bin1 = [bin1, filepath_1, '--auto', '--silent',
                      '--cpu-limit={0}'.format(timeout),
                      '--detsort-rw', '--detsort-new']
      bin1_indexed_res  = run_e(args_bin1)

      args_bin2  = [bin2, filepath_2] + args_bin1[2:]
      bin2_indexed_res = run_e(args_bin2)

      report.append({
        'filename': basename(filepath), 
        'theorem status': exp_status,
        'runtime bin1': bin1_indexed_res[1], 
        'theorem status bin1': bin1_indexed_res[0],
        'runtime bin2': bin2_indexed_res[1], 
        'theorem status bin2': bin2_indexed_res[0]
      })
      
      if bin1_indexed_res[0].strip() != bin2_indexed_res[0].strip():
        print('Theorem status difference: {0}.'.format(basename(filepath)))

  columns = ['filename', 'theorem status', 
            'runtime bin1', 'theorem status bin1', 
            'runtime bin2', 'theorem status bin2']
  print_report_csv(report, columns, 'two_bins_app_non_app.csv')


def usage(prog_name):
  print("python {0} <e binary> <e_binary> <test folder> <test_folder> <limit = None / number> <timeout>".format(prog_name))


def main():
  import sys

  if len(sys.argv) < 7:
    usage(sys.argv[0])
  else:

    bin1 = sys.argv[1]
    bin2 = sys.argv[2]
    test1 = sys.argv[3]
    test2 = sys.argv[4]
    limit = None if sys.argv[5] == "None" else int(sys.argv[5])
    timeout = int(sys.argv[6])

    cmp_binaries(bin1, bin2, test1, test2, limit, timeout)


if __name__ == '__main__':
  main()
