from subprocess import Popen, PIPE
import re, shlex

REGEX_SZS_STATUS =  re.compile(r"# SZS status (\S+)")
REGEX_TIME = re.compile(r"# Total time\s+:\s+(\S+)")
REGEX_EXPECTED   = re.compile(r"%\s+Status\s+[:]\s+?(.*)\b")

def usage(script):
  import sys
  msg = "Usage: python3 {0} <configuration_file> <timeout> <problems_root> <out_file>".\
         format(script)
  print(msg)
  sys.exit()


def get_exitcode_stdout_stderr(cmd):
  """
  Execute the external command and get its exitcode, stdout and stderr.
  """
  args = shlex.split(cmd)

  proc = Popen(args, stdout=PIPE, stderr=PIPE)
  out, err = proc.communicate()
  exitcode = proc.returncode
  #
  return exitcode, out.decode('utf-8'), err.decode('utf-8')


def process_out(out, timeout, probpath, conf, f):
  with open(probpath, 'r') as prob:
  	contents = prob.read()
  	match = REGEX_EXPECTED.findall(contents)
  	expected = '?'
  	if len(match):
  	  expected = match[0]

  status = REGEX_SZS_STATUS.findall(out)[0]
  if status != 'ResourceOut':
    time = REGEX_TIME.findall(out)[0]
  else:
    time = "{0}".format(timeout)

  f.write('{0}, {1}, {2}, {3}, {4}\n'.format(probpath, conf, status, time, expected))
  return status


def print_progress_bar (iteration, total, prefix = '', suffix = '', decimals = 2, length = 100, fill = '█'):
  percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
  filledLength = int(length * iteration // total)
  bar = fill * filledLength + ' ' * (length - filledLength)
  print('\r{0} |{1}| {2}% {3}'.format(prefix, bar, percent, suffix), end = '\r')
  # Print New Line on Complete
  if iteration == total: 
    print()



def main():
  import sys
  try:
    conf = sys.argv[1]
    timeout = int(sys.argv[2])
    root = sys.argv[3]
    out_path  = sys.argv[4]
    statuses  = {}
  except:
    usage(sys.argv[0])

  with open(out_path, "w") as out_f:
    out_f.write("problem, configuration, result, time, expected\n")

    all_probs = []

    import os
    for dirpath, _, filenames in os.walk(root):
      for filename in filter(lambda x: x.endswith(".p"), filenames):
        all_probs.append(os.path.join(dirpath, filename))

    prob_no = len(all_probs)
    for i, probpath in enumerate(all_probs):
      print_progress_bar(i+1, prob_no)

      try:
        e_code, out, err = \
          get_exitcode_stdout_stderr(" ".join([conf, probpath, str(timeout)]))
      except FileNotFoundError:
        print('Configuration is not found. Try with "./{0}"'.format(conf))
        sys.exit(-1)
      if "# SZS" in out:
        status = process_out(out, timeout, probpath, conf, out_f)
        if status in statuses:
          statuses[status] += 1
        else:
          statuses[status] = 1
      else:
        print("# Error at {0} :stderr: [{1}]; ec: [{2}]".\
              format(probpath, err.strip(), e_code))


  print("# Summary for {0}: ".format(root))
  for key in statuses:
    print("# {0} : {1}".format(key, statuses[key]))

if __name__ == '__main__':
  main()
