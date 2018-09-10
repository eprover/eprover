import pandas as pd

PROBLEM_NAME = 'prob_name'  
BENCHMARK    = 'benchmark'

SEPARATOR    = ','

EXCEL_ON  = True
EXCEL_OFF = False

ADD_OVER = True
ADD_DIFF = True
ADD_SUCCESS = True

STATUS_THEOREM          = 'Theorem'
STATUS_CONTRADICTORY_AX = 'ContradictoryAxioms'
STATUS_OUT_OF_RESOURCES = 'ResourceOut'
STATUS_TIMEOUT          = 'Timeout'
STATUS_UNSAT            = 'Unsatisfiable' 
STATUS_SAT              = 'Satisfiable'
STATUS_GAVE_UP          = 'GaveUp'
STATUS_COUNTERSAT       = 'CounterSatisfiable'

COMPLETED_STAT = [STATUS_THEOREM, STATUS_UNSAT, STATUS_SAT, STATUS_COUNTERSAT,STATUS_CONTRADICTORY_AX]
FAILED_STAT    = [STATUS_GAVE_UP, STATUS_OUT_OF_RESOURCES, STATUS_TIMEOUT]

def get_prob_name(full_name):
  sep = '/' if '/' in full_name else '\\'
  parts = full_name.split(sep)
  root_folder = parts[1].replace(".app", "").replace("app", "")
  file_name   = parts[-1].replace(".app", "").replace("app", "")#[:-len("cnf.p")]

  return '/'.join([root_folder, file_name]) 


def read_benchmark(filename):
  df = pd.read_csv(filename, sep=SEPARATOR)
  df[PROBLEM_NAME] =  list(map(get_prob_name, df[BENCHMARK]))
  df = df.set_index(PROBLEM_NAME)
  return df


def join_bencharks(benchmarks, columns):
  probnames = set() 

  for b in benchmarks:
    probnames =  probnames.union(set(b.index))
    print(b)

  res_cols = [PROBLEM_NAME]
  for i in range(1, len(benchmarks)+1):
    for col in columns:
      res_cols.append('{0}_{1}'.format(i, col))

  res = {} 
  for res_col in res_cols:
    res[res_col] = []

  for probname in probnames:
    res[PROBLEM_NAME].append(probname)
    for i,benchmark in enumerate(benchmarks):
      for col in columns:
        to_append = benchmark.at[probname, col] if probname in benchmark.index else '???'
        res['{0}_{1}'.format(i+1, col)].append(to_append)

  total_probs = len(res[PROBLEM_NAME])
  if any(filter(lambda x: x.endswith("result"), res_cols)) and \
      any(filter(lambda x: x.endswith("status"), res_cols)):
    for i in range(1, len(benchmarks)+1):
      for j in filter(lambda x: x!=i, range(1, len(benchmarks)+1)):
        over = '{0}_over_{1}'.format(i,j)
        if ADD_OVER:
          res[over] = []
        diff = '{0}_diff_{1}'.format(i,j)
        if ADD_DIFF:  
          res[diff] = []
        for k in range(0, total_probs):
          if ADD_OVER:
            if res[str(i)+'_result'][k] in COMPLETED_STAT and \
                 (res[str(j)+'_result'][k] in FAILED_STAT or \
                    res[str(j)+'_status'][k].startswith("timeout")):
              res[over].append(1)
            else:
              res[over].append(0)

          if ADD_DIFF:
            if res[str(i)+'_result'][k] in COMPLETED_STAT and \
                res[str(j)+'_result'][k] in COMPLETED_STAT and \
                  res[str(i)+'_result'][k] != res[str(j)+'_result'][k]:
              res[diff].append(1)
            else:
              res[diff].append(0)
      if ADD_SUCCESS:
        res[str(i)+'_success'] = list(map(lambda x: 1 if x in COMPLETED_STAT else 0, 
                                          res[str(i)+'_result']))
  else:
    print('warning: to determine prover over prover and prover diffs' +
          ' include result and status columns.')

  res = pd.DataFrame(res)

  print(res)
  return pd.DataFrame(res)


def output_joined(res, out_file, excel = EXCEL_OFF):
  if excel:
    writer = pd.ExcelWriter(out_file)
    res.to_excel(writer, 'Results', index=False)
    writer.save()
  else:
    res.to_csv(out_file, index=False)


def usage():
  print('<script> <b=benchmark1,benchark2, ...> <c=col1, col2, ...> <o=out_file>')
  print('  b is the list of benchmarks to join')
  print('  c is the list of columns to keep')

def clean_arg(arg):
  arg = arg.strip()
  if arg[1]=='=':
    arg = arg[2:]

  return arg


def main():
  import sys

  try:
    benchmarks = list(map(clean_arg, next(filter(lambda x: x.startswith("b="), sys.argv)).split(',')))
    columns    = list(map(clean_arg, next(filter(lambda x: x.startswith("c="), sys.argv)).split(',')))
    out_name   = next(filter(lambda x: x.startswith("o="), sys.argv))[2:].strip()

    excel_stat = EXCEL_ON if out_name.endswith("xlsx") else EXCEL_OFF

    output_joined(join_bencharks(list(map(read_benchmark, benchmarks)), columns), out_name, excel_stat)
  except Exception:
    usage()
    import traceback
    traceback.print_exc()

if __name__ == '__main__':
  main()
