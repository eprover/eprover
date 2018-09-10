from sys import argv
from os.path import isdir, isfile, join
from os import walk
import pandas as pd
import numpy as np

STATUS_THEOREM          = 'Theorem'
STATUS_CONTRADICTORY_AX = 'ContradictoryAxioms'
STATUS_OUT_OF_RESOURCES = 'ResourceOut'
STATUS_TIMEOUT          = 'Timeout'
STATUS_UNSAT            = 'Unsatisfiable' 
STATUS_SAT              = 'Satisfiable'
STATUS_GAVE_UP          = 'GaveUp'
STATUS_COUNTERSAT       = 'CounterSatisfiable'
STATUS_INAPPROPRIATE    = 'Inappropriate'

COMPLETED_STAT = [STATUS_THEOREM, STATUS_UNSAT, STATUS_SAT, STATUS_COUNTERSAT,STATUS_CONTRADICTORY_AX]
FAILED_STAT    = [STATUS_GAVE_UP, STATUS_OUT_OF_RESOURCES, STATUS_TIMEOUT, STATUS_INAPPROPRIATE]
ALL_STAT       = COMPLETED_STAT + FAILED_STAT


def get_files(root):
  res = []
  for (path, _, files) in walk(root):
    res += list(map(lambda f: join(path, f), 
                filter(lambda f: f.endswith(".csv"), files)))
  return res

def str2int(str):
  try:
    return int(str)
  except:
    return -1


def summarize(csvs):
  summaries = ALL_STAT + ['Success', 'Total cpu', 'Total cpu success', 'Avg success', 'Avg all solved']
  for csv in csvs:
    print("# summarizng " + csv)
    res = {}
    res['summary'] = summaries
    df = pd.read_csv(csv)
    max_solvers = max(map(lambda x: str2int(x.split('_')[0]), 
                          df.columns[:-1]))

    all_solved = pd.Series([True]*5012)
    for solver in range(1, max_solvers+1):
      result = str(solver) + "_result"
      status = str(solver) + "_status"

      all_solved = all_solved & df[result].isin(COMPLETED_STAT) & (df[status] == 'complete')

    print("# {0} problems were solved by all provers.".format(sum(all_solved)))


    for solver in range(1, max_solvers+1):
      solver = str(solver)
      prover, configuration = df[solver+"_solver"][0], df[solver+"_configuration"][0]
      label = "{0}_{1}".format(prover, configuration)
      res[label] = []

      result = solver + "_result"
      status = solver + "_status"
      cpu_time = solver + "_cpu time"
      for stat in ALL_STAT:
        res[label].append(len(df[df[result] == stat]))
      
      res[label].append(len(df[df[result].isin(COMPLETED_STAT) & (df[status] == 'complete')]))
      succ_count = sum(df[cpu_time])
      res[label].append(succ_count)
      total_cpu_succ = sum(df[df[result].isin(COMPLETED_STAT) & (df[status] == 'complete')][cpu_time])
      res[label].append(total_cpu_succ)
      res[label].append(total_cpu_succ / succ_count)
      res[label].append(sum(df[all_solved][cpu_time])*1.0/sum(all_solved))
    
    res_df = pd.DataFrame(res)
    print(res_df)
    res_df.to_csv(csv.replace(".csv", "summary.csv"), index=False)
    

def usage(script):
  print('python {0} <root or csv>'.format(script))
  print('  if directory is given as root, all .csv files will be processed')
  print('  in the hierarchy; if .csv file is given, only that file will')
  print('  be processed')

def main():
  if len(argv) > 1:
    path = argv[1]
    if isfile(path):
      to_summarize = [path]
    else:
      to_summarize = get_files(path)
    summarize(to_summarize)
  else:
    usage(argv[0])


if __name__ == '__main__':
  main()
