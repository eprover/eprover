def init_args():
  import argparse

  description = '''
    Analyzes schedule.vars and prints out the frequency analysis of
    configurations
  ''' 
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('schedule_vars_filename', metavar='SCHEDULE_VARS',
                      help='path to schedule.vars file')
  args = parser.parse_args()

  return args
  


def parse_schedule(sch_vars_path):
  import re
  SCH_VARS_REGEX_BEGIN = re.compile(r"ScheduleCell (preproc|search)_H\w+\s*\[\]\s*=\s*\{")
  SCH_VARS_CONF_LINE = re.compile(r"\{\s*\"([^\"]+?)\"[^}]*?\}")
  SCH_VARS_REGEX_END = re.compile(r"};")

  PREPROC_CONF, SEARCH_CONF, BETWEEN_CONFS = 0, 1, -1

  def update_sch_map(sch_map, conf_name, state):
    frequency = sch_map.get(conf_name, (0,0))
    if state == PREPROC_CONF:
      frequency = (frequency[0]+1, frequency[1])
    else:
      assert(state==SEARCH_CONF)
      frequency = (frequency[0], frequency[1]+1)
    sch_map[conf_name] = frequency


  sch_map = {}
  with open(sch_vars_path) as sch_vars_fd:
    file_end = False
    state = BETWEEN_CONFS

    while not file_end:
      line = sch_vars_fd.readline()
      if not line:
        file_end = True
      else:
        if state == BETWEEN_CONFS:
          match =  SCH_VARS_REGEX_BEGIN.search(line)
          if match and match.group(1) == "preproc":
            state = PREPROC_CONF
          elif match:
            state = SEARCH_CONF
        else:
          match = SCH_VARS_CONF_LINE.search(line)
          if match:
            conf = match.group(1)
            update_sch_map(sch_map, conf, state)
          else:
            match = SCH_VARS_REGEX_END.search(r"};")
            if match:
              state = BETWEEN_CONFS
    
    return sch_map


def print_analysis(freq_map):
  def print_ordered(projector):
    for conf, freq in sorted(freq_map.items(), key=lambda x: projector(x[1]), reverse=True):
      print(" > {0} : {1}".format(conf, projector(freq)))
  
  print("Preprocessing")
  print_ordered(lambda x: x[0])

  print("\nSearch")
  print_ordered(lambda x: x[1])


def main():
  args = init_args()
  print_analysis(parse_schedule(args.schedule_vars_filename))


if __name__ == "__main__":
  main()
