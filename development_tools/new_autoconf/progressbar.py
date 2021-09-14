def print_progress_bar (iteration, total, prefix = '', suffix = '', decimals = 2, length = 100, fill = '█'):
  percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
  filledLength = int(length * iteration // total)
  bar = fill * filledLength + ' ' * (length - filledLength)
  print('\r{0} |{1}| {2}% {3}'.format(prefix, bar, percent, suffix), end = '\r')
  # Print New Line on Complete
  if iteration == total: 
    print()
