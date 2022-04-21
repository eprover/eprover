
from common import Category, Configuration

def init_args():
  import argparse

  description = '''
    Processes the category file and displays statistical info
  ''' 
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('category_file', metavar='CATEGORY_ROOT',
                      help='file of containing lines of the form file:category')
  parser.add_argument('--less-than', dest='lt', type=int,
                      help='show the number of classes that have less than X problems')

  return parser.parse_args()

import pickle
def main():
  args = init_args()
  all_cats = {}
  with open(args.category_file) as fd:
    for line in fd:
      prob,cat_name = line.split(':', maxsplit=2)
      if cat_name not in all_cats:
        all_cats[cat_name] = [prob]
      else:
        all_cats[cat_name].append(prob)
  
  for (cat, probs) in sorted(all_cats.items(), 
                             key=lambda x: len(x[1]), reverse=True):
    print('{0}({1}): '.format(cat.strip(), len(probs)))
    print(" > {0}".format(", ".join(probs)))
  
  if args.lt:
    print("{0} / {1}".format(len(list(filter(lambda x:len(x[1]) < args.lt, all_cats.items()))), 
                            len(all_cats)))



if __name__ == '__main__':
  main()

