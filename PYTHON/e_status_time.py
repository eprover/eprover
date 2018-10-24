#!/usr/bin/python3

from e_evaluator import *

def test_status(binary, test_folder, timeout=60):
  import os

  report = []

  for root, dir, files in os.walk(test_folder):
    for file in files:
      filepath = os.path.join(root, file)

      exp_status = get_theorem_status(filepath)
        
      args = [binary, filepath, '--auto', '--silent',
              '--cpu-limit={0}'.format(timeout)]
      res  = run_e(args)

      report.append({
        'filename': file, 
        'theorem status': exp_status,
        'runtime': res[1], 
        'E status report': res[0],
      })

  columns = ['filename', 'theorem status', 
            'runtime', 'E status report']
  print_report_csv(report, columns, 'index_report.csv')


def usage(prog_name):
  print("python {0} <e binary> <test folder> [timeout]".format(prog_name))


def main():
  import sys

  if len(sys.argv) < 3:
    usage(sys.argv[0])
  else:
    test_status(sys.argv[1], sys.argv[2], 
                  int(sys.argv[3]) if len(sys.argv) >= 4 else 10)


if __name__ == '__main__':
  main()
