#!/usr/bin/python3

from e_evaluator import *
ERRORS_ONLY = False


def print_errors(binary, test_folder, timeout=60):
  import os

  report = []

  for root, dir, files in os.walk(test_folder):
    for file in filter(lambda x: x.endswith(".p"), files):
      filepath = os.path.join(root, file)

      exp_status = get_theorem_status(filepath)
        
      args = [binary, filepath, '--auto', '--silent',
              '--cpu-limit={0}'.format(timeout)]
      res  = run_e(args, print_status=True)

      # if errors_only then an error has to be present.
      if not ERRORS_ONLY or any(res[2].strip()):
        report.append({
          'filename': file, 
          'theorem status': exp_status,
          'runtime': res[1], 
          'E status report': res[0],
          'errors': res[2]
        })

      if exp_status != res[0]:
        print("Theorem status difference in {0} ({1}/{2})".format(filepath, 
                                                                  exp_status, res[0]))

  columns = ['filename', 'theorem status', 
            'runtime', 'E status report', 'errors']
  print_report_csv(report, columns, 'errors.csv')


def usage(prog_name):
  print("python {0} <e binary> <test folder> [timeout]".format(prog_name))


def main():
  import sys

  if len(sys.argv) < 3:
    usage(sys.argv[0])
  else:
    print_errors(sys.argv[1], sys.argv[2], 
                  int(sys.argv[3]) if len(sys.argv) >= 4 else 10)


if __name__ == '__main__':
  main()
