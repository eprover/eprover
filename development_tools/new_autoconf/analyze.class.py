
from common import Configuration, Category, ArchiveFormat

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
  parser.add_argument('category_file', metavar='CATEGORY_ROOT',
                      help='file containing list of files in the given category')
  parser.add_argument('--archive-format', dest='arch_format',
                      type=ArchiveFormat,
                      default=ArchiveFormat.JOBINFO_FORMAT,
                      choices=list(ArchiveFormat),
                      help='are the archives in the protocol format or in StarExec jobinfo format')
  args = parser.parse_args()

  return args

def main():
  args = init_args()
  from scheduler import parse_configurations
  configurations = parse_configurations(args.result_archives, args.arch_format).values()
  
  from os.path import basename
  cat = Category(basename(args.category_file))
  with open(args.category_file) as fd:
    for line in fd:
      cat.add_prob(line.strip())
  
  for conf in configurations:
    conf.evaluate_category(cat, configurations)
  
  def order(c, configurations, cat):
    eval = c.evaluate_category(cat, configurations)
    return (len(eval[0]), eval[1], -eval[2])

  for conf in sorted(configurations,
                     key=lambda x: order(x, configurations, cat)):
    eval = conf.evaluate_category(cat, configurations)
    print("{0} : {1},{2},{3}".format(conf.get_name(), len(eval[0]), eval[1], -eval[2]))

if __name__ == '__main__':
  main()

