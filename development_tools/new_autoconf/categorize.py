import os
import os.path as p

PROB_CATEGORIES_FILENAME = "problem_categories"
PROB_RAW_CATEGORIES_FILENAME = "problem_raw_categories"

CATEGORIZATIONS = [PROB_CATEGORIES_FILENAME, PROB_RAW_CATEGORIES_FILENAME]

UNPARSABLE_CLASS = 'unparsable_class'
TIMEOUT_CLASS = 'timeout_class'
ERROR_CLASS = 'error_class'

IGNORE_CLASSES = [UNPARSABLE_CLASS, TIMEOUT_CLASS, ERROR_CLASS]

class Classifier(object):
  def __init__(self, binary, opts, timeout):
    self._bin = binary
    self._opts = opts
    self._to = timeout

  def __call__(self, prob_path):
    import subprocess as sp
    try:
      proc_res = sp.run([self._bin, prob_path] + self._opts, 
                        stdout=sp.PIPE, stderr=sp.PIPE, 
                        timeout=self._to)
      # 5 when options are used
      if(proc_res.returncode in [0,5]):
        class_ = proc_res.stdout\
                         .decode(encoding='ascii', errors='ignore')\
                         .split(":")[-1]\
                         .strip()
        return (prob_path, class_)
      else:
        return (prob_path, UNPARSABLE_CLASS)
    except sp.TimeoutExpired:
      return (prob_path, TIMEOUT_CLASS)
    except:
      import traceback
      return (prob_path, "ERROR_CLASS:{0}".format(traceback.format_exc()))


def get_probs(root, prob_filter):
  all_files = []
  for (path, _, files) in os.walk(root):
    files =\
      map(lambda f: p.join(path, f),
        filter(lambda f: f.endswith(".p") and
                (prob_filter is None or 
                 any(filter(lambda c: c in f, prob_filter))), 
               files))
    all_files += files
  return all_files


def make_class_map(probs, e_classify_bin, e_classify_args, 
                   max_cpus, mask, raw_mask, timeout):
  c_maps = []
  p_maps = []

  for extra_args in (([] if not mask else ["--class-mask="+mask]), 
                    (["--raw-class"] + ([] if not raw_mask else ["--raw-mask="+raw_mask]))):
    try:
      import multiprocessing as m
      pool = m.Pool(max_cpus)
      classifier = Classifier(e_classify_bin, e_classify_args + extra_args, timeout)
      classified = pool.imap_unordered(classifier, probs)

      class_map = {}
      prob_map = {}
      total_probs = len(probs)
      for (i, (prob, class_)) in enumerate(classified):
        import progressbar as pb
        pb.print_progress_bar(i+1, total_probs)
        
        if(class_.startswith("ERROR_CLASS:")):
          # print('{0}: {1}'.format(prob, class_))
          class_ = ERROR_CLASS
        
        prob = p.basename(prob)

        if(len(class_) == 0):
          print("empty: {0}".format(prob))

        if class_ not in class_map:
          class_map[class_] = [prob]
        else:
          class_map[class_].append(prob)
        
        prob_map[prob] = class_
      
      # return (class_map, prob_map)
      c_maps.append(class_map)
      p_maps.append(prob_map)
    except:
      import traceback
      print("% Fatal error:{0}".format(traceback.format_exc()))
  return (c_maps, p_maps)


def make_class_dir(class_maps, prob_maps, out_dir, e_args, raw_mask, mask):
  os.makedirs(out_dir, exist_ok=True)

  for (i, cat_name) in enumerate(CATEGORIZATIONS):
    with open(p.join(out_dir, cat_name), 'w') as fd:
      for (prob, class_) in prob_maps[i].items():
        fd.write("{0}:{1}\n".format(prob, class_))
  
  with open(p.join(out_dir, "description"), 'w') as fd:
    fd.write("The following (non-default) parameters were used:\n")
    fd.write(','.join(e_args) + "\n")
    if raw_mask:
      fd.write("raw_mask: -c{0}\n".format(raw_mask))
    if mask:
      fd.write("mask: -c{0}\n".format(mask))

  for c_map in class_maps:
    for(class_, probs) in c_map.items():
      with open(p.join(out_dir, class_), 'w') as fd:
        for prob in probs:
          fd.write(prob + '\n')


def init_args():
  import argparse

  description = '''
    Processes the given TPTP hierarchy by applying e_classify on each problem
    to obtain the class. Then, a directory is made where each file
    has the name of the class and contains all the problems that belong to this
    class.
  ''' 
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('e_classify_path', metavar='E_CLASSIFY_PATH',
                      help='path to the e_classify binary')
  parser.add_argument('root', metavar='ROOT',
                      help='root directory containing TPTP problem files')
  parser.add_argument('e_classify_args', metavar='E_CLASSIFY_ARGS', nargs='*',
                      help='arguments passed to e_classify')
  parser.add_argument('--filter', dest='filter', default=None,
                      help='take problems that have any of '
                           'the characters given in filter')
  parser.add_argument('--mask', dest='mask', default=None,
                      help='mask for post-CNF clausifier')
  parser.add_argument('--raw-mask', dest='raw_mask', default=None,
                      help='mask for pre-CNF clausifier')
  parser.add_argument('--out-dir', dest='out_dir', default="classes",
                      help='name of the directory where class files are stored')
  parser.add_argument('--max-cpus', dest='max_cpus', default=os.cpu_count(), type=int,
                      help='maximal number of CPUs to be used for '
                           'categorization. The default is all available CPUs ')
  parser.add_argument('--binary-timeout', dest='timeout', default=60, type=int,
                      help='timeout for a single e_classify run')
  args = parser.parse_args()

  return args


def main():
  args = init_args()
  all_probs = get_probs(args.root, args.filter)
  print("Found {0} problems.".format(len(all_probs)))
  class_map, prob_map = make_class_map(all_probs, args.e_classify_path,
                                       args.e_classify_args, args.max_cpus,
                                       args.mask, args.raw_mask,
                                       args.timeout)
  print("There are {0}/{1} classes.".format(len(class_map[0]), len(class_map[1])))
  make_class_dir(class_map, prob_map, args.out_dir, args.e_classify_args, 
                 args.raw_mask, args.mask)

if __name__ == '__main__':
  main()
