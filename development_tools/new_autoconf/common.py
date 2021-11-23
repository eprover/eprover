
def tuple_is_smaller(a, b):
  a,b = list(a[:3]),list(b[:3])
  a[2],b[2] = -a[2], -b[2] # smaller times are better
  return a < b #lexicographic comp

class Category(object):
  def __init__(self, name):
    self._name = name
    self._probs = []
    self._best = None

  def add_prob(self, prob):
    self._probs.append(prob)

  def get_problems(self):
    return self._probs

  def get_name(self):
    return self._name

  def store_evaluation(self, eval, conf):
    if self._best is None:
      self._best = (eval[0], eval[1], eval[2], conf)
    else:
      if tuple_is_smaller(self._best, eval):
        self._best = (eval[0], eval[1], eval[2], conf)

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

  PREPROCESSING_KEYS = [
    'no_preproc', 'eqdef_maxclauses', 'eqdef_incrlimit',
    'formula_def_limit',  'sine', 'presat_interreduction',
    'lift_lambdas', 'lambda_to_forall', 'unroll_only_formulas'
  ]

  ONLY_PREPROCESSING = 1
  ONLY_SATURATION = 2
  BOTH = 3

  def __init__(self, name):
    self._name = name
    self._probs = {}
    self._memo_eval = {}
    self._num_solved = 0
    self._num_attempted = 0
    self._total_time = 0.0
    self._total_uniqness = None
    self._json = None
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
    self._num_attempted += 1
    if prob not in Configuration._all_attempted:
      Configuration._all_attempted[prob] = 1
    else:
      Configuration._all_attempted[prob] += 1

  def evaluate_for_probs(self, probs):
    solved, uniqness_points, time = (0, 0, 0.0)
    for prob in probs:
      if prob in self._probs:
        solved += 1
        uniqness_points += Configuration._all_attempted[prob] - Configuration._all_solved[prob]
        time += self._probs[prob]
    return (solved, uniqness_points, time / max(solved,1))

  def evaluate_category(self, category):
    if category in self._memo_eval:
      return self._memo_eval[category]   
    
    eval = self.evaluate_for_probs(category.get_problems())
    self._memo_eval[category] = eval
    category.store_evaluation(eval, self)
    return eval

  def rate_general(self):
    if self._total_uniqness is None:
      self._total_uniqness = 0
      for prob in self._probs.keys():
        self._total_uniqness += Configuration._all_attempted[prob] - Configuration._all_solved[prob]

    return (self._num_solved / self._num_attempted, self._total_uniqness,
            -self._total_time / max(self._num_solved,1))


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

    if json_mode == self.BOTH:
      json = self._json
    elif json_mode == self.ONLY_PREPROCESSING:
      lines = filter(lambda l: '{' in l or '}' in l or \
                               any(filter(lambda k: l.strip().startswith(k), 
                                          self.PREPROCESSING_KEYS)),
                     self._json.split('\n'))
      json = '\n'.join(lines)
    else:
      assert(json_mode == self.ONLY_SATURATION)
      lines = filter(lambda l: '{' in l or '}' in l or \
                               all(filter(lambda k: not l.strip().startswith(k), 
                                          self.PREPROCESSING_KEYS)),
                     self._json.split('\n'))
      json = '\n'.join(lines)
    return '#{0}\\n{1}'.format(self._name, json.replace("\n", "\\n").replace('"', '\\"'))
    

  def get_name(self):
    from pathlib import Path
    return Path(self._name).stem

  def get_solved_probs(self):
    return set(self._probs.keys())
  
  def __str__(self):
    return "{0} : ({1}, {2})".format(self._name, self._num_solved, 
                                     self._total_time)

  def __repr__(self):
    return str(self)
