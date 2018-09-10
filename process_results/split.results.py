import pandas as pd


SEPARATOR = ','


EXCEL_ON  = True
EXCEL_OFF = False


def output_joined(res, out_file, excel = EXCEL_OFF):
  if excel:
    writer = pd.ExcelWriter(out_file)
    res.to_excel(writer, 'Results', index=False)
    writer.save()
  else:
    res.to_csv(out_file, index=False, sep=SEPARATOR)


def split_benchmarks(filename, base, bench_df, split_col):
  import os.path as p

  print(bench_df)

  unique_vals = list(bench_df[split_col].unique())

  if (len(unique_vals) > 200):
    print('Too many unique values ({0})'.format(unique_vals))
    return

  for unique_val in unique_vals:
    filtered = bench_df[bench_df[split_col] == unique_val]
    name = p.join(base, "{1}{0}.csv".format(str(unique_val).replace(" ","_"), 
    			                                  filename.replace("csv", "")))
    output_joined(filtered, name)


def main():
  import sys

  if len(sys.argv) >= 3:
    benchmark = sys.argv[1]
    split_col = sys.argv[2]
    base_folder = '.' if len(sys.argv) == 3 else sys.argv[3]
    split_benchmarks(benchmark, base_folder,
                     pd.read_csv(benchmark, sep=SEPARATOR), split_col)
  else:
    print("python3 {0} <benchmark> <col_to_split> [out_folder]".format(sys.argv[0]))
     


if __name__ == '__main__':
  main()
