
from common import Category, Configuration

def init_args():
  import argparse

  description = '''
    Processes the given TPTP hierarchy by applying e_classify on each problem
    to obtain the class. Then, a directory is made where each file
    has the name of the class and contains all the problems that belong to this
    class.
  ''' 
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('category_file', metavar='CATEGORY_ROOT',
                      help='file of containing lines of the form file:category')
  parser.add_argument('target_category', metavar='TARGET_CATEGORY',
                      help='the category to analyze')
  parser.add_argument('--result-archives', dest='result_archives', nargs='+',
                      help='archives containing the evaluation results')
  parser.add_argument('--conf-root', dest='conf_root',
                      help='root directory containing JSON files corresponding to configurations in JobInfo files')
  parser.add_argument('--pickled-configurations', dest='pickled_confs',
                      help='path to the pickled configurations dict so that we do '
                           'not have to parse configurations every time')
  parser.add_argument('--e-path', dest='e_path',
                      help='path to eprover which is necessary for some features of this script (e.g., '
                         ' generation of JSON representation  of the configuration)')
  parser.add_argument("--single-problem", dest='single_prob',default=False, action='store_true')

  args = parser.parse_args()
  if args.pickled_confs is None and\
     not (args.result_archives is not None and\
          args.conf_root is not None and args.e_path is not None):
    parser.error('Either pickle a confs map or parse'
                 ' the result archives with the corresponding conf root') 

  return args

import pickle
def main():
  args = init_args()
  from scheduler import parse_configurations
  if args.pickled_confs is None:
    confs_dict = parse_configurations(args.result_archives, args.e_path, 100, args.conf_root)
    configurations = list(confs_dict.values())
    with open('pickled_confs.txt', 'wb') as pickle_fd:
      pickle.dump(configurations, pickle_fd)
      pickle.dump(Configuration._all_solved, pickle_fd)
      pickle.dump(Configuration._all_attempted, pickle_fd)
      pickle.dump(Configuration._all_confs, pickle_fd)

  else:
    print("\rParsing pickled configurations...", end="\r")
    with open(args.pickled_confs, 'rb') as pickle_fd:
      configurations = pickle.load(pickle_fd)
      Configuration._all_solved = pickle.load(pickle_fd)
      Configuration._all_attempted = pickle.load(pickle_fd)
      Configuration._all_confs = pickle.load(pickle_fd)
    print("\rPickled configurations parsed.    ")

  
  from os.path import basename
  name = "single" if args.single_prob else basename(args.target_category) 
  cat = Category(name)
  if not args.single_prob:
    with open(args.category_file) as fd:
      for line in fd:
        prob,cat_name = line.split(':', maxsplit=2)
        if cat_name.strip() == name:
          cat.add_prob(prob)
  else:
    cat.add_prob(args.target_category.strip())
  
  for conf in configurations:
    conf.evaluate_category(cat)
  
  for conf in sorted(configurations, key=lambda x: x.evaluate_category(cat)):
    eval = conf.evaluate_category(cat)
    print("{0} : {1}|{2}|{3}|{4}".format(conf.get_name(), eval[0], eval[1], eval[2], eval[3]))

  from scheduler import schedule
  print(cat)
  # multi_schedule(8, [cat], configurations, "TEST", Configuration.BOTH, dbg=True)
  schedule([cat], configurations, 3, 10, set(), True, 0.1, dbg=True)


if __name__ == '__main__':
  main()

