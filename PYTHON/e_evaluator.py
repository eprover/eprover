def run_e(args, stdin_file = None, print_status = False):
  import time as t
  import subprocess as sp
  import os

  start = t.time()
  out = None

  if print_status:
    print("Working on {0}".format(args[1]))

  with open(".e_tmp_err", "w") as tmp_err:
    try:
      res = sp.run(args, stdout = sp.PIPE, stderr=tmp_err)\
        if stdin_file is None else sp.run(args, stdin = stdin_file, stdout = sp.PIPE, stderr=tmp_err)
      out = res.stdout
    except sp.CalledProcessError as e:
      out = b'# SZS status hoE crashed'
      print('SubprocessError: {0}'.format(e))
    end   = t.time()

  with open(".e_tmp_err", "r") as tmp_err:
    err = tmp_err.read()

  os.remove(".e_tmp_err")

  out = out.decode('utf-8')
  try:
    szs_line = next(filter(lambda x: 'SZS status' in x, out.split('\n')))
    szs_line = szs_line.replace("# SZS status", "")
  except StopIteration:
    szs_line = 'status line not found in the output'

  return (szs_line.strip(), end-start, err.strip())


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
      out.write(",".join(['"{0}"'.format(str(line[col]) if col in line else 'n/a') \
                                         for col in columns]) + "\n")


