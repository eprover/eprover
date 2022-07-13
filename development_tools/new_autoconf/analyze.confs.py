
from common import Configuration

def init_args():
  import argparse

  description = '''
    Processes the given TPTP hierarchy by applying e_classify on each problem
    to obtain the class. Then, a directory is made where each file
    has the name of the class and contains all the problems that belong to this
    class.
  ''' 
  parser = argparse.ArgumentParser(description=description)
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
    confs_dict = parse_configurations(args.result_archives, args.e_path, args.conf_root)
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

  
  for conf in sorted(configurations, key=lambda x: x.rate_general(), reverse=True):
    print("{0} : {1}/{2}".format(conf.get_name(), 
                                 len(conf.get_solved_probs()),
                                 conf.get_num_attempted()))

if __name__ == '__main__':
  main()

