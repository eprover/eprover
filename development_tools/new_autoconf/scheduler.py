import os.path as p
from sys import stderr
from common import Category, Configuration, ArchiveFormat, tuple_is_smaller

JOBINFO_PROB_NAME = 'benchmark'
JOBINFO_CONFIGURATION = 'configuration'
JOBINFO_TIME = 'cpu time'
JOBINFO_RESULT = 'result'

SUCCESS_RESULTS = ['ContradictoryAxioms', 'Theorem', 'Unsatisfiable']

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

    
def parse_configurations(archives, archive_format, json_root=None):
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
      print("Warning: {0} is not appropriate StarExec JobInfo archive".format(arch),
            file=stderr)
  if json_root is not None:
    for conf in confs.values():
      conf.parse_json(p.join(json_root, conf.get_name()))

  return confs


def pop_best(confs, cats):
  best_stat = (set(), -1, 0.0, None)
  for conf in confs:
    stat = (set(), 0, 0.0, conf)
    for cat in cats:
      # the first time evaluate_category is called with all the
      # confs and this is going to be cached
      cat_stat = conf.evaluate_category(cat, confs)
      stat = (stat[0].union(cat_stat[0]), stat[1] + cat_stat[1], 
              stat[2] + cat_stat[2], conf)
    from common import tuple_is_smaller
    if tuple_is_smaller(best_stat, stat):
      best_stat = stat
  confs.remove(best_stat[3])
  return best_stat[3]

def cover_cats(conf, cats, others):
  covered = set()
  for cat in cats:
    if (conf.evaluate_category(cat, others)[0] and
        cat.get_best_conf() == conf):
      print("conf({0}) = {1}".format(cat, conf.get_name()), file=stderr)
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
      conf.evaluate_category(cat, confs)

  cat_to_confs = {}

  while confs and cats:
    best_conf = confs.popleft() if take_general else pop_best(confs, cats)
    covered_cats = cover_cats(best_conf, cats, confs)
    for cat in covered_cats:
      cat_to_confs[cat.get_name()] = best_conf
    cats = cats.difference(covered_cats)
  
  if cats:
    print("unassinged: {0}".format(",".join(map(str, cats))), file=stderr)
  
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
