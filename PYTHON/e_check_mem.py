import sys

def calc_mem(args, stdin_file = None, print_status = False):
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
    malloced = next(filter(lambda x: 'SizeMalloc()ed' in x, out.split('\n')))
    freed    = next(filter(lambda x: 'SizeFree()ed' in x, out.split('\n')))

    malloced = int(malloced.split(":")[1].strip().split(" ")[0])
    freed    = int(freed.split(":")[1].strip().split(" ")[0])
  except StopIteration:
    malloced, freed = (None, None)
    szs_line = 'status line not found in the output'

  return (malloced, freed)

def test_mem(bin1, test_folder, limit=None, timeout=60):
  report = []

  from os import listdir
  from os.path import isfile, join, basename
  file_names = [f for f in listdir(test_folder) if isfile(join(test_folder, f))]

  import random as rnd
  rnd.shuffle(file_names)

  if limit==None:
    limit = len(file_names)

  for (i,filepath) in enumerate(file_names[:limit]):
      filepath_1 = join(test_folder, filepath)
        
      with open(filepath_1, "r") as in_file:
         args_bin1 = [bin1, filepath_1, '--auto', '--silent',
                         '--soft-cpu-limit={0}'.format(timeout)]
         (allocd, freed) = calc_mem(args_bin1, in_file)
      leaked = allocd - freed if allocd is not None else None

      print("{0} : alloc = {1}, leaked = {2}, percent = {3}"\
               .format(filepath, allocd, leaked, (leaked/allocd)*100) if leaked is not None else "{0}:err".format(filepath))


def usage(prog_name):
   print("python {0} <e binary> <e_binary> <test folder> <test_folder> <limit = None / number> <timeout>".format(prog_name))


def main():
   bin1 = sys.argv[1]
   test = sys.argv[2]
   limit = None if sys.argv[3] == "None" else int(sys.argv[5])
   timeout = int(sys.argv[4])

   test_mem(bin1, test, limit, timeout)


if __name__ == '__main__':
  main()

