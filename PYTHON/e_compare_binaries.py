#!/usr/bin/python3

from e_evaluator import *

import re

FOF_REGEX = r'[A-Z]{3}\d{3}(_|\+|\-]).*\.p$'
FOF_AUTOMATON = re.compile(FOF_REGEX)

def is_fof(file_name):
  return any(FOF_AUTOMATON.finditer(file_name.strip()))

def cmp_binaries(bin1, bin2, test_folder, limit=None, timeout=60):
  import os

  report = []

  file_names = []

  for root, dir, files in os.walk(test_folder):
    for file in filter(is_fof, files):
      file_names.append(os.path.join(root, file))

  import random as rnd
  rnd.shuffle(file_names)

  if limit==None:
    limit = len(file_names)

  for (i,filepath) in enumerate(file_names[:limit]):
      print('Working on {0}/{1}'.format(i+1, limit))
      exp_status = get_theorem_status(filepath)
        
      args_bin1 = [bin1, filepath, '--auto', '--silent',
                      '--cpu-limit={0}'.format(timeout)]
      bin1_indexed_res  = run_e(args_bin1)

      args_bin2  = [bin2] + args_bin1[1:]
      bin2_indexed_res = run_e(args_bin2)

      report.append({
        'filename': os.path.basename(filepath), 
        'theorem status': exp_status,
        'runtime bin1': bin1_indexed_res[1], 
        'theorem status bin1': bin1_indexed_res[0],
        'runtime bin2': bin2_indexed_res[1], 
        'theorem status bin2': bin2_indexed_res[0]
      })
      
      if bin1_indexed_res[0].strip() != bin2_indexed_res[0].strip():
        print('Index state difference result: {0}.'.format(file))

  columns = ['filename', 'theorem status', 
            'runtime bin1', 'theorem status bin1', 
            'runtime bin2', 'theorem status bin2']
  print_report_csv(report, columns, 'two_bins.csv')


def usage(prog_name):
  print("python {0} <e binary> <e_binary> <test folder> [limit] [timeout]".format(prog_name))


def main():
  import sys

  if len(sys.argv) < 4:
    usage(sys.argv[0])
  else:
    cmp_binaries(sys.argv[1], sys.argv[2], sys.argv[3],
                  int(sys.argv[4]) if len(sys.argv) >= 5 else None,
                  int(sys.argv[5]) if len(sys.argv) >= 6 else 35)


if __name__ == '__main__':
  main()
