#!/usr/bin/env python3

"""
scheduler.py

Usage:


Example: scheduler.py --min-preproc-size=3 --max-preproc-size=8 --min-search-size=4 --max-search-size=12 --e-path ~/SOURCES/Projects/E/PROVER/eprover-ho ~/EPROVER/PETAR_TUNING/*.zip ~/EPROVER/CLASSES_NEWCONFIG  --min-cat-size=3 > ~/schedule.vars


Options:

-h  Print this help.

--conf-root CONF_ROOT]
                    [--max-preproc-size MAX_PREPROC_SIZE]
                    [--min-preproc-size MIN_PREPROC_SIZE]
                    [--min-search-size MIN_SEARCH_SIZE]
                    [--max-search-size MAX_SEARCH_SIZE]
                    [--min-cat-size MIN_CAT_SIZE] [--e-path E_PATH]
                    RESULT_ARCHIVES [RESULT_ARCHIVES ...] CATEGORY_ROOT


Copyright 2022 Petar Vukmirovic and Stephan Schulz

This code is part of the support structure for the equational
theorem prover E. Visit

 http://www.eprover.org

for more information.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program ; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston,
MA  02111-1307 USA

The original copyright holder can be contacted as

Stephan Schulz
DHBW Stuttgart
Informatik
Jaegerstrasse 56
70174 Stuttgart
Germany

or (preferably) via email at schulz@eprover.org
"""

import enum
import os.path as p
from sys import stderr
from typing import ChainMap
from common import Category, Configuration, tuple_is_smaller
from progressbar import print_progress_bar

JOBINFO_PROB_NAME = 'benchmark'
JOBINFO_CONFIGURATION = 'configuration'
JOBINFO_TIME = 'cpu time'
JOBINFO_RESULT = 'result'

SUCCESS_RESULTS = ['ContradictoryAxioms', 'Theorem', 'Unsatisfiable']

PROTOCOL = 'protocol_'

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

def parse_jobinfo_file(_, fd, confs, __, limit):
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
        if time < limit:
          conf.add_solved_prob(prob, time)
        else:
          conf.add_attempted_prob(prob)
      else:
        conf.add_attempted_prob(prob)
    return True
  except (csv.Error, KeyError):
    return False


def parse_protocol_file(filename, fd, confs, e_path, limit):
  try:
    assert(filename.startswith(PROTOCOL))
    first_line = next(fd)
    if not first_line.startswith('#'):
      raise StopIteration

    e_args = first_line[1:].strip().split(' ')[1:-1]
    e_args.append("/Users/schulz/dummy.p")
    from pathlib import Path
    conf_name = Path(filename[len(PROTOCOL):]).stem

    if conf_name in confs:
      conf = confs[conf_name]
    else:
      conf = Configuration(conf_name)
      confs[conf_name] = conf
    conf.compute_json(e_path, e_args)

    columns = {}
    import re
    col_re = re.compile(r'#\s*?(\d+)\s*(.*)')

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
      try:
        (prob, status, time) = values[columns[PROBLEM_COL]], values[columns[STATUS_COL]],\
                               values[columns[TIME_COL]]
        if (status != 'F'):
          try:
            if float(time) < limit:
              conf.add_solved_prob(prob, float(time))
            else:
              conf.add_attempted_prob(prob)
          except ValueError:
            print('# Error with line {0} in {1}'.format(line, filename), file=stderr)
            conf.add_attempted_prob(prob)
        else:
          conf.add_attempted_prob(prob)
      except KeyError:
        print('# Error with line {0} in {1}'.format(line, filename), file=stderr)
    return True
  except StopIteration:
    print('Iteration stopped')
    return False


def parse_configurations(archives, e_path, limit, json_root=None):
  confs = {}

  def parse_file(file_name, file_descriptor):
    from pathlib import Path
    file_name = Path(file_name).name
    return (parse_protocol_file if file_name.startswith(PROTOCOL) else parse_jobinfo_file)(
      file_name, file_descriptor, confs, e_path, limit
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
  if json_root:
    for conf in Path(json_root).iterdir():
      conf_name = Path(conf).name
      if conf_name in confs and confs[conf_name].to_json() is None:
        confs[conf_name].parse_json(conf)

  return confs


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
                      help='root directory containing JSON files corresponding '
                           'to configurations in JobInfo files')
  parser.add_argument('--max-preproc-size', dest='max_preproc_size', default=4, type=int,
                      help='maximal size of preprocessing schedule')
  parser.add_argument('--min-preproc-size', dest='min_preproc_size', default=1, type=int,
                      help='minimal size of preprocessing schedule')
  parser.add_argument('--min-search-size', dest='min_search_size', default=3, type=int,
                      help='minimal size of search schedule')
  parser.add_argument('--max-search-size', dest='max_search_size', default=6, type=int,
                      help='maximal size of search schedule')
  parser.add_argument('--min-cat-size', dest='min_cat_size', type=int,
                      help='ignore categories that have less than X elements')
  parser.add_argument('--e-path', dest='e_path',
                      help='path to eprover which is necessary for some features of this script (e.g.,'
                           ' generation of JSON representation  of the configuration)')
  args = parser.parse_args()

  return args


def get_best_conf(confs, last, probs):
  m_conf, m_eval = None, (-1,-1,0,0,0)
  for c in confs:
    eval = c.evaluate_for_probs(last, probs)
    if tuple_is_smaller(m_eval, eval):
      m_conf = c
      m_eval = eval

  return (m_conf, m_eval)

def adjust_ratios(schedule, min_ratio):
  total_ratio = sum(map(lambda x: x[1], schedule))
  assert(total_ratio <= 1.02) #around 1.0 due to cutoff cannot be so precise.

  if total_ratio <= 0.98:
    mult = 1 / total_ratio
    schedule = list(map(lambda x: (x[0], mult*x[1]), schedule))

  min_ratio = min(min_ratio, 1/len(schedule))
  schedule.sort(key=lambda x: x[1], reverse=True)

  i=len(schedule)-1
  fix_amount = 0
  while i>=0 and schedule[i][1] < min_ratio :
    (conf, ratio) = schedule[i]
    fix_amount += min_ratio - ratio
    schedule[i] = (conf, min_ratio)
    i-=1

  divider = i+1;
  while i>=0:
    to_remove = fix_amount / divider
    (conf, ratio) = schedule[i]
    if(ratio-to_remove) < min_ratio:
      schedule[i] = (conf, min_ratio)
      divider -= 1
      fix_amount -= ratio - min_ratio
    else:
      schedule[i] = (conf, ratio-to_remove)
      fix_amount -= to_remove
    i -= 1

  return schedule

def schedule(cats, confs, min_size, max_size, used_confs,
             unique_preproc=False, min_ratio=0.05, dbg=False):
  cats = cats.values() if type(cats) is dict else cats
  confs = confs.values() if type(confs) is dict else confs

  res = {}
  n = len(cats)
  for (i,cat) in enumerate(cats):
    print_progress_bar(i, n)

    schedule = []
    remaining_probs = set(cat.get_problems())
    total_probs = len(remaining_probs)
    sched_size = 0
    remaining_ratio = 1
    remaining_confs = set(confs)
    raw_confs = set(confs)

    while remaining_confs and remaining_probs and sched_size<max_size:
      best_conf, best_eval = get_best_conf(remaining_confs,
                                           schedule[-1][0] if schedule else None,
                                           remaining_probs)
      if best_eval[0] == 0:
        #no problems can be solved by any of the remaining confs
        break
      else:
        solved_probs = remaining_probs.intersection(best_conf.get_solved_probs())
        if dbg:
          print('# {0} : {1}'.format(best_conf.get_name(), solved_probs))
        ratio = len(solved_probs)/total_probs
        remaining_ratio -= ratio
        schedule.append( (best_conf, ratio) )
        remaining_probs.difference_update(solved_probs)
        remaining_confs.remove(best_conf)
        raw_confs.remove(best_conf)
        if unique_preproc:
          same_preproc = set(filter(lambda x: x.get_preprocess_params()
                                  == best_conf.get_preprocess_params(),
                                remaining_confs))
          if (dbg):
            print("removing {0}".format(list(map(lambda x: x.get_name(), same_preproc))))
          remaining_confs.difference_update(same_preproc)

        sched_size += 1

    if sched_size<min_size:
      to_add = min_size - sched_size

      # if we did not have enough configurations to fill
      # in the scheule, then we take the best ones until
      # the schedule is filled

      assert(raw_confs)
      #assert(remaining_confs)

      def eval_probs(conf, last, cat):
        (sol, uniq, dist, succ, time) = conf.evaluate_for_probs(last, cat.get_problems())
        return (sol, uniq, dist, succ, -time)

      last = schedule[-1][0] if schedule else None
      remaining_confs = list(sorted(raw_confs, key=lambda x: eval_probs(x, last, cat),
                                    reverse=True))
      rem_ratio = remaining_ratio / to_add
      while remaining_confs and to_add:
        chosen_conf = remaining_confs[0]
        schedule.append( (chosen_conf, rem_ratio) )
        remaining_confs = list(filter(lambda x: x.get_preprocess_params()
                                                 != chosen_conf.get_preprocess_params(),
                              remaining_confs))
        to_add -= 1


    if dbg:
      final_sch =  list(map(lambda x: x[0], schedule))
      print("Final schedule: {0}".format(",".join(map(Configuration.get_name, final_sch))))
      print("Schedule preproc: {0}".format(";\n".join(list(map(lambda x: str(x.get_preprocess_params()), final_sch)))))

    used_confs.update(map(lambda x: x[0], schedule))
    res[cat] = adjust_ratios(schedule, min_ratio)

  return res

def output_used_confs(confs):
  print('StrStrPair conf_map[] =\n  {');
  for c in confs:
    print('    {{ "{0}", "{1}" }}, '.format(c.get_name(), c.to_json()))
  print('{ NULL, NULL }');
  print('};')
  print("const int num_confs = {0};".format(len(confs)))

def get_sched_name(var_prefix, cat):
  return var_prefix + "_" + cat.get_name().replace('-', '_')

def output_schedule(var_prefix, schedule, extra_field=False):
  SC = 'ScheduleCell'
  cat,confs_w_ratio = schedule
  print('{0} {1}[] = {{'.format(SC, get_sched_name(var_prefix, cat)))
  for (c,r) in confs_w_ratio:
    print ('{{ "{0}", NoOrdering, NULL, {1}, 1, 1 }}, '.format(c.get_name(), round(r,4)))
  if extra_field:
    print('{ "<placeholder>", NoOrdering, NULL, 0, 1, 1}, ')
  print('{ NULL, NoOrdering, NULL, 0, 1, 1}')
  print('};')


def output_schedule_map(var_prefix, schedule, extra_field=False):
  for sch in schedule.items():
    output_schedule(var_prefix, sch, extra_field)
  print('StrSchedPair {0}_sched_map[] =\n  {{'.format(var_prefix));
  for cat in schedule.keys():
    print('{{ "{0}", {1}, {2} }}, '.format(cat.get_name(), get_sched_name(var_prefix, cat),
                                           cat.get_size()))
  print('{ NULL, NULL, 0 }');
  print('};')
  print('const int num_{0} = {1};'.format(var_prefix, len(schedule)))


def clean_small_cats(cats, limit):
  return {k:v for (k,v) in cats.items() if len(v.get_problems()) >= limit}


def generate_best_schedule(cat_map, confs, min_size, max_size, min_ratio=None):
  if min_ratio is None:
    min_ratio = 1.0/max_size

  DUMMY_NAME = "DEFAULT_SCHEDULE"
  all_probs = Category(DUMMY_NAME)
  for prob_set in map(lambda x: x.get_problems(), cat_map.values()):
    for prob in prob_set:
      all_probs.add_prob(prob)

  return schedule({DUMMY_NAME: all_probs}, confs, min_size, max_size,
                  set(), min_ratio=min_ratio)


def main():
  args = init_args()
  category_map, raw_category_map = parse_categories(args.category_root)
  if args.min_cat_size:
    category_map = clean_small_cats(category_map, args.min_cat_size)
    raw_category_map = clean_small_cats(raw_category_map, args.min_cat_size)
  configurations = parse_configurations(args.result_archives, args.e_path, 100, args.conf_root)
  used_confs = set()

  preproc_sched = schedule(raw_category_map, configurations, args.min_preproc_size,
                           args.max_preproc_size, used_confs, True, 0.1)
  search_sched = schedule(category_map, configurations, args.min_search_size,
                          args.max_search_size, used_confs, min_ratio=0.1)
  best_sched = generate_best_schedule(category_map, configurations, 1, 8)

  print('// Found {0} confs, using {1}'.format(len(configurations), len(used_confs)))
  output_used_confs(used_confs)
  output_schedule_map("preproc", preproc_sched)
  output_schedule_map("search", search_sched, extra_field=True)
  output_schedule("", next(iter(best_sched.items())))

if __name__ == '__main__':
  main()
