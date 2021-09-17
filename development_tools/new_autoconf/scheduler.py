from enum import Enum
import os
import os.path as p

JOBINFO_PROB_NAME = 'benchmark'
JOBINFO_CONFIGURATION = 'configuration'
JOBINFO_TIME = 'cpu time'
JOBINFO_RESULT = 'result'

SUCCESS_RESULTS = ['ContradictoryAxioms', 'Theorem', 'Unsatisfiable']

class ArchiveFormat(Enum):
  PROTOCOL_FORMAT = 'protocol'
  JOBINFO_FORMAT = 'jobinfo'

  def __str__(self):
    return self.value


class Category(object):
  def __init__(self, name):
    self._name = name
    self._probs = []
    self._best = None
  
  def add_prob(self, prob):
    self._probs.append(prob)

  def get_problems(self):
    return self._probs

  def get_name(self):
    return self._name

  def store_evaluation(self, eval, conf):
    if self._best is None:
      self._best = (eval[0], eval[1], conf)
    else:
      curr = self._best[:2]
      if eval > curr:
        self._best = (eval[0], eval[1], conf)
  
  def get_best_conf(self):
    if self._best is None:
      return None
    else:
      return self._best[2]
  
  def __str__(self):
    return "{0} : {1}".format(self._name, ",".join(self._probs))

  def __repr__(self):
    return str(self)


class Configuration(object):
  def __init__(self, name):
    self._name = name
    self._probs = {}
    self._memo_eval = {}
    self._num_solved = 0
    self._total_time = 0.0

  def add_solved_prob(self, prob, time):
    self._num_solved += 1
    self._total_time += time
    self._probs[prob] = time

  def evaluate_category(self, category):
    if category in self._memo_eval:
      return self._memo_eval[category]
    
    solved, time = (0, 0.0)
    for prob in category.get_problems():
      if prob in self._probs:
        solved += 1
        time += self._probs[prob]
    self._memo_eval[category] = (solved, time)
    category.store_evaluation((solved, time), self)
    return (solved, time)
  
  def stats(self):
    return (self._num_solved, self._total_time)
  
  def parse_json(self, path):
    with open(path, 'r') as fd:
      self._json = fd.read().replace("\n", "\\n").replace('"', '\\"')

  def to_json(self):
    return self._json

  def get_name(self):
    from pathlib import Path
    return Path(self._name).stem


  def __str__(self):
    return "{0} : ({1}, {2})".format(self._name, self._num_solved, 
                                     self._total_time)

  def __repr__(self):
    return str(self)

def parse_categories(root):
  from categorize import CATEGORIZATIONS, IGNORE_CLASSES
  cats = []
  for cat_filename in CATEGORIZATIONS:
    f_name = p.join(root, cat_filename)
    if p.exists(root) and p.isdir(root) and\
       p.exists(f_name):
      category_map = {}
      with open(f_name) as fd:
        for line in fd:
          prob,cat_name = map(str.strip, line.split(":"))
          if cat_name not in IGNORE_CLASSES:
            if cat_name in category_map:
              category = category_map[cat_name]
            else:
              category = Category(cat_name)
              category_map[cat_name] = category
            category.add_prob(prob)
      cats.append(category_map)
    else:
      import sys
      print(("{0} is not appropriate TPTP categorization root. "+
            "Use categorize.py to create the catetgorization").format(root))
      sys.exit(-1)
  return cats


def parse_result_file(fd, confs):
  import csv
  reader = csv.DictReader(fd)

  for row in reader:
    result = row[JOBINFO_RESULT].strip()
    if result in SUCCESS_RESULTS:
      prob = row[JOBINFO_PROB_NAME].split('/')[-1].strip()
      conf_name = row[JOBINFO_CONFIGURATION].strip()
      time = float(row[JOBINFO_TIME].strip())

      if conf_name in confs:
        conf = confs[conf_name]
      else:
        conf = Configuration(conf_name)
        confs[conf_name] = conf
      
      conf.add_solved_prob(prob, time)

    
def parse_configurations(archives, archive_format, json_root):
  if(archive_format == ArchiveFormat.PROTOCOL_FORMAT):
    raise NotImplementedError
  
  confs = {}
  for arch in archives:
    from zipfile import ZipFile
    try:
      with ZipFile(arch) as arch_fd:
        import io
        csv_file = next(filter(lambda x: x.filename.endswith('.csv'), 
                               arch_fd.infolist()))
        with io.TextIOWrapper(arch_fd.open(csv_file.filename)) as csv_fd:
          parse_result_file(csv_fd, confs)
    except StopIteration:
      import sys
      print("Warning: {0} is not appropriate StarExec JobInfo archive".format(arch),
            file=sys.stderr)
  for conf in confs.values():
    conf.parse_json(p.join(json_root, conf.get_name()))

  return confs


def pop_best(confs, cats):
  best_stat = (-1, 0.0, None)
  for conf in confs:
    stat = (0, 0.0, conf)
    for cat in cats:
      cat_stat = conf.evaluate_category(cat)
      stat = (stat[0] + cat_stat[0], stat[1] + cat_stat[1], stat[2])
    if stat[:2] > best_stat[:2]:
      best_stat = stat
  confs.remove(best_stat[2])
  return best_stat[2]

def cover_cats(conf, cats):
  covered = set()
  for cat in cats:
    if (conf.evaluate_category(cat)[0] != 0 and
        cat.get_best_conf() == conf):
      covered.add(cat)
  return covered

def schedule(cats, confs, take_general):
  from collections import deque
  if take_general:
    confs = deque(sorted(confs.values(), key=Configuration.stats))
  else:
    confs = set(confs.values())
  cats = set(cats.values())

  # precomputation of the timing data for all confs and cats
  for conf in confs:
    for cat in cats:
      conf.evaluate_category(cat)

  cat_to_confs = {}

  while confs and cats:
    best_conf = confs.popleft() if take_general else pop_best(confs, cats)
    covered_cats = cover_cats(best_conf, cats)
    for cat in covered_cats:
      cat_to_confs[cat.get_name()] = best_conf
    cats.difference(covered_cats)
  
  return cat_to_confs


def init_args():
  import argparse

  description = '''
    Processes the given TPTP hierarchy by applying e_classify on each problem
    to obtain the class. Then, a directory is made where each file
    has the name of the class and contains all the problems that belong to this
    class.
  ''' 
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('result_archives', metavar='RESULT_ARCHIVES', nargs='+',
                      help='archives containing the evaluation results')
  parser.add_argument('category_root', metavar='CATEGORY_ROOT',
                      help='root directory containing classes of TPTP problems. '
                           'If the root directory does not exist or if it is not '
                           'of the correct form, it can be created using '
                           'categorize.py script. Consult the help of categorize.py '
                           'for more information.')
  parser.add_argument('conf_root', metavar='CONFIGURATION_ROOT',
                      help='root directory containing JSON files describing heuristics')
  parser.add_argument('--archive-format', dest='arch_format',
                      type=ArchiveFormat,
                      default=ArchiveFormat.JOBINFO_FORMAT,
                      choices=list(ArchiveFormat),
                      help='are the archives in the protocol format or in StarExec jobinfo format')
  parser.add_argument('--prefer-general', dest='prefer_general',
                    type=bool, default=False,
                    help='when two configurations perform the same on the given category of'
                         'problems prefer the one that performs better overall')
  args = parser.parse_args()

  return args

def output(confs, category_to_confs, raw_category_to_conf):
  best_conf = max(confs.values(), key=Configuration.stats)

  print('const long  num_categories = {0};'.format(len(category_to_confs)))
  print('const long  num_raw_categories = {0};'.format(len(raw_category_to_conf)))

  def conf_w_comment(conf):
    return '"{0}"/*{1}*/'.format(conf.to_json(), conf.get_name())

  print('const char* best_conf = {0};'.format(conf_w_comment(best_conf)))
  
  def print_str_list(var_name, str_list):
    print('const char* {0}[] = {{ '.format(var_name))
    print(",\n".join(str_list))
    print("};")

  print_str_list("categories", map(lambda x: '"{0}"'.format(x), 
                                   category_to_confs.keys()))
  print_str_list("confs", map(conf_w_comment, 
                              category_to_confs.values()))

  print_str_list("raw_categories", map(lambda x: '"{0}"'.format(x),
                                       raw_category_to_conf.keys()))
  print_str_list("raw_confs", map(conf_w_comment, 
                                  raw_category_to_conf.values()))


def main():
  args = init_args()
  category_map, raw_category_map = parse_categories(args.category_root)
  configurations = parse_configurations(args.result_archives, args.arch_format, args.conf_root)
  category_to_conf = schedule(category_map, configurations, args.prefer_general)
  raw_category_to_conf = schedule(raw_category_map, configurations, args.prefer_general)
  output(configurations, category_to_conf, raw_category_to_conf)



if __name__ == '__main__':
  main()
