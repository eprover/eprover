import enum
import os.path as p
from sys import stderr
from common import Category, Configuration, tuple_is_smaller
from progressbar import print_progress_bar

JOBINFO_PROB_NAME = 'benchmark'
JOBINFO_CONFIGURATION = 'configuration'
JOBINFO_TIME = 'cpu time'
JOBINFO_RESULT = 'result'

SUCCESS_RESULTS = ['ContradictoryAxioms', 'Theorem', 'Unsatisfiable']

PROTOCOL = 'protocol_'


def parse_categories(root):
  from categorize import IGNORE_CLASSES, PROB_CATEGORIES_FILENAME
  category_map = {}
  f_name = p.join(root, PROB_CATEGORIES_FILENAME)
  if p.exists(root) and p.isdir(root) and p.exists(f_name):
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
  else:
    import sys
    print(("{0} is not appropriate TPTP categorization root. "+
          "Use categorize.py to create the catetgorization").format(root))
    sys.exit(-1)
  return category_map

def parse_jobinfo_file(_, fd, confs, __):
  import csv, pathlib as p
  try:
    reader = csv.DictReader(fd)

    for row in reader:
      result = row[JOBINFO_RESULT].strip()
      prob = row[JOBINFO_PROB_NAME].split('/')[-1].strip()
      conf_name = p.Path(row[JOBINFO_CONFIGURATION].strip()).stem
      if conf_name in confs:
        conf = confs[conf_name]
      else:
        conf = Configuration(conf_name)
        confs[conf_name] = conf
      if result in SUCCESS_RESULTS:
        time = float(row[JOBINFO_TIME].strip())        
        conf.add_solved_prob(prob, time)
      else:
        conf.add_attempted_prob(prob)
    return True
  except (csv.Error, KeyError):
    return False


def parse_protocol_file(filename, fd, confs, e_path):
  try:
    assert(filename.startswith(PROTOCOL))
    first_line = next(fd)
    if not first_line.startswith('#'):
      raise StopIteration
    
    e_args = first_line[1:].strip().split(' ')[1:]
    conf_name = filename[len(PROTOCOL):]
    if conf_name in confs:
      print('{0} is ignored because it is already parsed'.format(filename), file=stderr)
      raise StopIteration

    columns = {}
    import re
    col_re = re.compile(r'#\s*?(\d+)\s*(.*)')

    conf = Configuration(conf_name)
    conf.compute_json(e_path, e_args)

    PROBLEM_COL = 'Problem'
    STATUS_COL = 'Status'
    TIME_COL =  'User time'

    line = next(fd)
    while True:
      m = col_re.match(line)
      if m:
        columns[m.group(2).strip()] = int(m.group(1))-1
      else:
        break
      line = next(fd)
    next(fd) # ignoring line with all columns

    for line in fd:
      values = line.split()
      (prob, status, time) = values[columns[PROBLEM_COL]], values[columns[STATUS_COL]],\
                             values[columns[TIME_COL]]
      if (status != 'F'):
        try:
          conf.add_solved_prob(prob, float(time))
        except ValueError:
          print('# Error with line {0} in {1}'.format(line, filename), file=stderr)
          conf.add_attempted_prob(prob)
      else:
        conf.add_attempted_prob(prob)

    confs[conf.get_name()] = conf
    return True
  except StopIteration:
    print('Iteration stopped')
    return False


def parse_configurations(archives, e_path, json_root=None):
  confs = {}

  def parse_file(file_name, file_descriptor):
    from pathlib import Path
    file_name = Path(file_name).name
    return (parse_protocol_file if file_name.startswith(PROTOCOL) else parse_jobinfo_file)(
      file_name, file_descriptor, confs, e_path
    )

  def parse_zip(arch):
    any_success = False
    from zipfile import ZipFile
    with ZipFile(arch) as fd:
      import io
      names = list(filter(lambda x: x.endswith('.csv'), fd.namelist()))
      for (i,csv_filename) in enumerate(names):
        print_progress_bar(i+1, len(names))
        with io.TextIOWrapper(fd.open(csv_filename)) as csv_fd:
          any_success = parse_file(csv_filename, csv_fd) or any_success
    return any_success
  
  import tarfile as tf
  def parse_tar(arch):
    any_success = False
    with tf.open(arch, mode='r') as fd:
      import io
      for (i,csv_member) in enumerate(fd):
        msg = f'\r --> file {i}: {csv_member.name}|'
        print(f'{msg:<150}', end = '\r', file=stderr)
        if csv_member.name.endswith('csv'):
          with io.TextIOWrapper(fd.extractfile(csv_member)) as csv_fd:
            any_success = parse_file(csv_member.name, csv_fd) or any_success
    return any_success

  for arch in archives:
    print('Working on {0}'.format(arch), file=stderr)
    success = (parse_tar if tf.is_tarfile(arch) else parse_zip)(arch)
    if not success:
      print('WARNING: No configurations could be parsed from {0}'.format(arch), file=stderr)

  from pathlib import Path
  for conf in Path(json_root).iterdir():
    conf_name = Path(conf).name
    if conf_name in confs and confs[conf_name].to_json() is None:
      confs[conf_name].parse_json(conf)
  
  return confs


def pop_best(confs, cats):
  best_stat = (0, -1, 0.0, None)
  for conf in confs:
    stat = (0, 0, 0.0, conf)
    for cat in cats:
      # the first time evaluate_category is called with all the
      # confs and this is going to be cached
      cat_stat = conf.evaluate_category(cat)
      stat = (stat[0] + cat_stat[0], stat[1] + cat_stat[1], 
              stat[2] + cat_stat[2], conf)
    if tuple_is_smaller(best_stat, stat):
      best_stat = stat
  confs.remove(best_stat[3])
  return best_stat[3]


def cover_cats(conf, cats, others):
  covered = set()
  for cat in cats:
    if (conf.evaluate_category(cat)[0] and
        cat.get_best_conf() == conf):
      covered.add(cat)
  return covered


def schedule_best_single(cats, confs, take_general):
  from collections import deque
  if take_general:
    confs = deque(sorted(confs.values(), key=Configuration.rate_general, reverse=True))
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
    covered_cats = cover_cats(best_conf, cats, confs)
    for cat in covered_cats:
      cat_to_confs[cat.get_name()] = best_conf
    cats = cats.difference(covered_cats)
    
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
  parser.add_argument('--conf-root', dest='conf_root',
                      help='root directory containing JSON files corresponding to configurations in JobInfo files')
  parser.add_argument('--e-path', dest='e_path',
                    help='path to eprover which is necessary for some features of this script (e.g., '
                         ' generation of JSON representation  of the configuration)')
  parser.add_argument('--prefer-general', dest='prefer_general',
                    type=bool, default=False,
                    help='when two configurations perform the same on the given category of'
                         'problems prefer the one that performs better overall')
  parser.add_argument("--multi-schedule", dest='multi_schedule', default=[], nargs='+', type=float,
                      metavar='TIME_RATIO',
                      help='instead of a single autoconfiguration, create a schedule of confiurations;'
                           ' a list of ratios of time limits (at least two elements) is'
                           ' specified as the argument. For example if argument is [0.33, 0.33, 0.33] each '
                           ' of the generated configurations will be run for a third of the time.')
  args = parser.parse_args()
  if args.multi_schedule and len(args.multi_schedule) < 2:
    parser.error("--multi-schedule requires at least two arguments")
  
  if args.multi_schedule and sum(args.multi_schedule) < 0.98:
    parser.error("--multi-schedule arguments must sum up to (approximately) 1")

  return args


def print_str_list(var_name, str_list, type_modifier = "const char*", array_modifier="[]"):
    print('{1} {0}{2} = {{ '.format(var_name, type_modifier, array_modifier))
    print(",\n".join(str_list))
    print("};")


def output_single(confs, category_to_confs):
  best_conf = max(confs.values(), key=Configuration.rate_general)

  print('const long  num_categories = {0};'.format(len(category_to_confs)))

  def conf_w_comment(conf, json_kind=Configuration.BOTH):
    return '"{0}"/*{1}*/'.format(conf.to_json(json_kind), conf.get_name())

  print('const char* best_conf = {0};'.format(conf_w_comment(best_conf)))

  cat_keys, cat_vals = list(zip(*category_to_confs.items()))
  print_str_list("categories", map(lambda x: '"{0}"'.format(x), cat_keys))
  print_str_list("confs", 
    map(lambda c: conf_w_comment(c, Configuration.ONLY_SATURATION), cat_vals))


def print_new_schedule_cell(time_ratios):
  print("#define SCHEDULE_SIZE {0}".format(len(time_ratios)))
  print("ScheduleCell NEW_HO_SCHEDULE[] =\n{");
  for (i,ratio) in enumerate(time_ratios):
    print('  {{ "AutoNewSched_{0}", NoOrdering, "Auto",  {1:.2f}, 0}},'.format(i, ratio))
  print('  {NULL, NoOrdering, NULL, 0.0, 0} ')
  print('};')


def output_multi_schedule(cat_to_conf, sched_size, cat_name, conf_name, json_kind):
  cat_keys, schedules = list(zip(*cat_to_conf.items()))
  print('const int {0}_len = {1};'.format(cat_name, len(cat_keys)))
  print_str_list(cat_name, map(lambda c: '"' + c.get_name() + '"', cat_keys))
  print_str_list(conf_name, ['{' +  ",".join(['"' + c.to_json(json_kind) + '"' for c in s]) + '}' for s in schedules],
                 array_modifier='[][{0}]'.format(sched_size))


def eval2key(x):
  return (x[0], x[1], -x[2])

def multi_schedule(num_confs, cats, confs, var_name, json_kind):
  assert(num_confs >= 2)
  assert(len(confs) >= num_confs)
  cats = cats.values()
  confs = confs.values()
  
  #precomputation
  for conf in confs:
    for cat in cats:
      conf.evaluate_category(cat)

  res = {}
  for cat in cats:
    best_for_cat = cat.get_best_conf()
    sched_size = 1
    schedule = [best_for_cat]
   
    remaining_probs = set(cat.get_problems())
    for conf in schedule:
      remaining_probs = remaining_probs.difference(conf.get_solved_probs())
    remaining_confs = set(confs).difference(schedule)
      
    while remaining_probs and sched_size!=num_confs:
      best_conf = max(remaining_confs,
                      key=lambda x: eval2key(x.evaluate_for_probs(remaining_probs)))
      curr_res = best_conf.evaluate_for_probs(remaining_probs)
      if curr_res[0] == 0:
        #no problems can be solved by any of the remaining confs
        break
      else:
        schedule.append(best_conf)
        remaining_probs = remaining_probs.difference(best_conf.get_solved_probs())
        remaining_confs.remove(best_conf)
        sched_size += 1
    if sched_size!=num_confs:
      schedule += list(sorted(remaining_confs, 
                              key=lambda x: eval2key(x.evaluate_category(cat)), 
                              reverse=True))[:num_confs-sched_size]
    
    assert(len(schedule) == num_confs)

    res[cat] = schedule
  
  output_multi_schedule(res, num_confs, var_name, 
                        var_name.replace('categories', 'confs'),
                        json_kind)


def schedule_multiple(time_ratios, cats, confs):
  print_new_schedule_cell(time_ratios)
  multi_schedule(len(time_ratios), cats, confs, "multischedule_categories", Configuration.ONLY_SATURATION)


def main():
  args = init_args()
  category_map = parse_categories(args.category_root)
  configurations = parse_configurations(args.result_archives, args.e_path, args.conf_root)
  if not args.multi_schedule:
    category_to_conf = schedule_best_single(category_map, configurations, args.prefer_general)
    output_single(configurations, category_to_conf)
  else:
    schedule_multiple(args.multi_schedule, category_map, configurations)

if __name__ == '__main__':
  main()
