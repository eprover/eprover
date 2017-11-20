def run_e(args, stdin_file = None):
  import time as t
  import subprocess as sp

  start = t.time()
  try:
    out = sp.check_output(args) if stdin_file is None \
                                else sp.check_output(args, stdin = stdin_file)
  except sp.CalledProcessError:
    out = b'# SZS status hoE crashed'
  end   = t.time()

  out = out.decode('utf-8')
  try:
    szs_line = next(filter(lambda x: 'SZS status' in x, out.split()))
    szs_line = szs_line.replace("# SZS status", "")
  except StopIteration:
    szs_line = 'status line not found in the output'

  return (szs_line, end-start)


def get_theorem_status(problem_filepath):
  with open(problem_filepath, "r") as problem_file:
    for line in problem_file:
      if '% Status' in line:
        return line.split(':')[1].strip()


import sys
def print_report_csv(report, columns = None, outpath = sys.stdout):
  if columns is None:
    assert(len(report) > 0)
    columns = list(report[0].iterkeys())

  with open(outpath, "w") as out:
    out.write(",".join(columns) + "\n")

    for line in report:
      out.write(",".join([line[col] if col in line else 'n/a' \
                         for col in columns]) + "\n")


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
                  int(sys.argv[3]) if len(sys.argv) >= 4 else 60)


if __name__ == '__main__':
  main()