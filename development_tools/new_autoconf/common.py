
def tuple_is_smaller(a, b):
  a,b = list(a[:5]),list(b[:5])
  a[4],b[4] = -a[4], -b[4] # smaller times are better

  return a < b

class Category(object):
  def __init__(self, name):
    self._name = name
    self._probs = []
    self._best = None

  def add_prob(self, prob):
    self._probs.append(prob)

  def get_problems(self):
    return self._probs

  def get_size(self):
    return len(self.get_problems())

  def get_name(self):
    return self._name

  def store_evaluation(self, eval, conf):
    if self._best is None:
      self._best = (eval[0], eval[1], eval[2], eval[3], eval[4], conf)
    else:
      if tuple_is_smaller(self._best, eval):
        self._best = (eval[0], eval[1], eval[2], eval[3], eval[4], conf)

  def get_best_conf(self):
    if self._best is None:
      return None
    else:
      return self._best[3]

  def __str__(self):
    return "{0} : {1}".format(self._name, ",".join(self._probs))

  def __repr__(self):
    return str(self)


class Configuration(object):
  _all_solved = {} # hold info how many confs solved a problem
  _all_attempted = {} # hold info how many confs attempted a problem
  _all_confs = 0  # how many objetcs of Conf type have been created

  PREPROCESSING_W_DEFAULTS =\
  {
    # 'no_preproc': 'false',
    # 'eqdef_maxclauses': '20000',
    # 'eqdef_incrlimit': '20',
    'formula_def_limit': '24',
    'sine': 'Auto',
    'lift_lambdas': 'true',
    'lambda_to_forall': 'true',
    'unroll_only_formulas' : 'true',
    'inst_choice_max_depth' : '-1',
    'preinstantiate_induction' : 'false'
  }

  ONLY_PREPROCESSING = 1
  ONLY_SATURATION = 2
  BOTH = 3

  def __init__(self, name):
    self._name = name
    self._probs = {}
    self._attempted = set()
    self._memo_eval = {}
    self._num_solved = 0
    self._total_time = 0.0
    self._total_uniqness = None
    self._json = None
    self._preprocess = None
    Configuration._all_confs += 1

  def add_solved_prob(self, prob, time):
    self._num_solved += 1
    self._total_time += time
    self._probs[prob] = time
    if prob not in Configuration._all_solved:
      Configuration._all_solved[prob] = 1
    else:
      Configuration._all_solved[prob] += 1
    self.add_attempted_prob(prob)

  def remove_solved_over_time(self, max_time):
    self._probs = {p:t for (p,t) in self._probs.items() if t < max_time}
    self._total_time = sum(self._probs.values())
    self._num_solved = len(self._probs)

  def add_attempted_prob(self, prob):
    self._attempted.add(prob)
    if prob not in Configuration._all_attempted:
      Configuration._all_attempted[prob] = 1
    else:
      Configuration._all_attempted[prob] += 1

  def evaluate_for_probs(self, prev_conf, probs):
    solved, uniqness_points, time = (0, 0, 0.0)
    for prob in probs:
      if prob in self._probs:
        solved += 1
        uniqness_points += Configuration._all_attempted[prob] - Configuration._all_solved[prob]
        time += self._probs[prob]
    # order of rating: 1) solved for problem list
    # 2) score of uniqueness within the solved problems
    # 3) how different this configuration is with respect to the
    # previous one (for preprocessing)
    # 4) ration of solved to attempted problems,
    # 5) avg necessary time to solve a prob
    return (solved, uniqness_points,
            0 if prev_conf is None else self._num_diffs(prev_conf),
            len(self._probs) / max(len(self._attempted), 1),
            time / max(solved,1))

  def evaluate_category(self, category):
    if category in self._memo_eval:
      return self._memo_eval[category]

    eval = self.evaluate_for_probs(None, category.get_problems())
    self._memo_eval[category] = eval
    category.store_evaluation(eval, self)
    return eval


  def parse_json(self, path):
    with open(path, 'r') as fd:
      import re
      self._json = re.sub(' +', ' ', fd.read()) # making the representation more compact

  def compute_json(self, eprover_path, args):
    import subprocess as sp, re
    JSON_PRINT = ['--print-strategy']

    proc_res = sp.run(args=[eprover_path] + args + JSON_PRINT, capture_output=True)
    raw_res = proc_res.stdout.decode(encoding='ascii', errors='ignore')
    self._json = re.sub(' +', ' ', raw_res)

  def to_json(self, json_mode=BOTH):
    if self._json is None:
      return None

    json = self._json
    return '#{0}\\n{1}'.format(self._name,
                               json.replace('"', '\\"').replace("\n", "\\n\"\n\"  "))

  def get_preprocess_params(self):
    if self._preprocess is not None:
      return self._preprocess

    if self._json is None:
      return None

    self._preprocess = {}
    for key,def_val in Configuration.PREPROCESSING_W_DEFAULTS.items():
      json_lines = self._json.split('\n')
      for line in json_lines:
        if key in line:
          self._preprocess[key] = line.split(':')[1].strip()
          break
      if key not in self._preprocess:
        self._preprocess[key] = def_val

    return self._preprocess

  def _num_diffs(self, other):
    diffs = 0
    for key in Configuration.PREPROCESSING_W_DEFAULTS.keys():
      diffs += self._preprocess.get(key, None) != other._preprocess.get(key, None)
    return diffs

  def get_name(self):
    from pathlib import Path
    return Path(self._name).stem

  def get_solved_probs(self):
    return set(self._probs.keys())

  def get_num_attempted(self):
    return len(self._attempted)

  def __str__(self):
    return "{0} : ({1}, {2})".format(self._name, self._num_solved,
                                     self._total_time)

  def __repr__(self):
    return str(self)
