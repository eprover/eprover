class Configuration:
  import re
  CPU_LIMIT_REGEX = re.compile(r"--cpu-limit=\d+")
  PROB_NAME_REGEX = re.compile(r"(([/][^/]*)*([^/]+[.]p))")
  EPROVER_REGEX = re.compile(r"((./)\s*)?eprover(-fo|-ho)?")

  SCRIPT_TEXT = '''#!/bin/tcsh
echo -n "% Problem    : " ; head -2 $1 | tail -1 | sed -e "s/.*  : //"
set args="{0}"
set final="--cpu-limit="$STAREXEC_CPU_LIMIT" "$1
set ecmd="./eprover $args $final"

echo "% Command    : " $ecmd
/home/starexec/bin/GetComputerInfo -p THIS Model CPUModel RAMPerCPU OS | \
    sed -e "s/Computer     /% Computer   /" \
        -e "s/Model        /% Model      /" \
        -e "s/CPUModel     /% CPU        /" \
        -e "s/RAMPerCPU    /% Memory     /" \
        -e "s/OS           /% OS         /"
echo -n "% CPULimit   : " ; echo "$STAREXEC_CPU_LIMIT"
echo -n "% DateTime   : " ; date
echo "% CPUTime    : "
$ecmd  
'''

  def __init__(self, line, name):
    self._line = line
    self._name = name

  def get_argument_line(self):
    l = self._line.strip("# ")
    l = self.CPU_LIMIT_REGEX.sub("", l)
    l = self.EPROVER_REGEX.sub("", l)
    l = filter(lambda part: not self.PROB_NAME_REGEX.match(part), l.split())

    return " ".join(l)
  
  def get_name(self):
    cleaned_name =  self._name.replace(".csv", "").replace(".sh", "").\
                               replace("starexec_run_", "").\
                               replace("protocol_", "")
    from os.path import basename
    return basename(cleaned_name)

  def print_script(self, root_folder):
    import os.path as p
    with open(p.join(root_folder, self.get_name()), "w") as fd:
      fd.write(self.SCRIPT_TEXT.format(self.get_argument_line()))
  
  class ParseException(Exception):
    pass


def init_args():
  import argparse

  description = '''
    Gets archives with protocol files and extracts starexec configurations 
    that would result in the same configuration that is described in the 
    protocol file.
  ''' 
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('protocol_archives', metavar='PROTOCOL_ARCHIVES', nargs='+',
                      help='archives containing the protocol files')
  parser.add_argument('output_root', metavar='OUTPUT_ROOT',
                      help='root directory where the output scripts are to be stored')
  args = parser.parse_args()

  return args

def parse_file(name, fd):
  first_line = next(fd)
  return Configuration(first_line, name)

def is_protocol_file(name):
  import os.path as p
  fname = p.basename(name)
  return fname.startswith("protocol_") and fname.endswith(".csv")

def parse_zip(arch):
  parsed_confs = []
  from zipfile import ZipFile
  with ZipFile(arch) as fd:
    import io
    names = list(filter(is_protocol_file, fd.namelist()))
    for csv_filename in names:
      with io.TextIOWrapper(fd.open(csv_filename)) as csv_fd:
        try:
          parsed_confs.append(parse_file(csv_filename, csv_fd))
        except Configuration.ParseException:
          pass
  return parsed_confs

from genericpath import exists
import tarfile as tf
def parse_tar(arch):
  parsed_confs = []
  with tf.open(arch, mode='r') as fd:
    import io
    for csv_member in fd:
      if is_protocol_file(csv_member.name):
        with io.TextIOWrapper(fd.extractfile(csv_member)) as csv_fd:
          try:
            parsed_confs.append(parse_file(csv_filename, csv_fd))
          except Configuration.ParseException:
            pass
  return parsed_confs

def parse_confs(archives):
  confs_by_name = {}
  for archive in archives:
    if ".tar" in archive:
      confs_in_archive = parse_tar(archive)
    elif ".zip" in archive:
      confs_in_archive = parse_zip(archive)
    else:
      confs_in_archive = []
      print('Warning: {0} is not a supported archive'.format(archive))

    for c in confs_in_archive:
      if c.get_name() not in confs_by_name:
        confs_by_name[c.get_name()] = c
      else:
        prev_conf = confs_by_name[c.get_name()]
        if prev_conf.get_argument_line() != c.get_argument_line():
          print('Warning: {0} of archive {1} is ignored, as it was previously defined'.\
                format(c.get_name(), archive))

  return confs_by_name.values()

def output_confs(confs, output):
  import os
  os.makedirs(output, exist_ok=True)

  for c in confs:
    c.print_script(output)


def main():
  args = init_args()
  output_confs(parse_confs(args.protocol_archives), args.output_root)


if __name__ == "__main__":
  main()
