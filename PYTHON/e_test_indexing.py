#!/usr/bin/python3

from e_evaluator import *

def test_indexing(binary, test_folder, timeout=60):
  import os

  report = []

  for root, dir, files in os.walk(test_folder):
    for file in files:
      filepath = os.path.join(root, file)

      exp_status = get_theorem_status(filepath)
        
      args_indexed = [binary, filepath, '--auto', '--silent',
                      '--cpu-limit={0}'.format(timeout)]
      indexed_res  = run_e(args_indexed)

      args_non_indexed  = args_indexed + \
                          ['--pm-into-index=NoIndex', '--pm-from-index=NoIndex']
      non_indexed_res = run_e(args_non_indexed)

      report.append({
        'filename': file, 
        'theorem status': exp_status,
        'runtime indexed': indexed_res[1], 
        'theorem status indexed': indexed_res[0],
        'runtime non-indexed': non_indexed_res[1], 
        'theorem status non-indexed': non_indexed_res[0]
      })

      if indexed_res[0].strip() != non_indexed_res[0].strip():
        print('Index state difference result: {0}.'.format(file))

  columns = ['filename', 'theorem status', 
            'runtime indexed', 'theorem status indexed', 
            'runtime non-indexed', 'theorem status non-indexed']
  print_report_csv(report, columns, 'index_report.csv')


def usage(prog_name):
  print("python {0} <e binary> <test folder> [timeout]".format(prog_name))


def main():
  import sys

  if len(sys.argv) < 3:
    usage(sys.argv[0])
  else:
    test_indexing(sys.argv[1], sys.argv[2], 
                  int(sys.argv[3]) if len(sys.argv) >= 4 else 10)


if __name__ == '__main__':
  main()
