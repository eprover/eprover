
def tuple_is_smaller(a, b):
  a,b = list(a[:3]),list(b[:3])
  a[2],b[2] = -a[2], -b[2] # smaller times are better
  return a < b #lexicographic comp

from enum import Enum
class ArchiveFormat(Enum):
  PROTOCOL_FORMAT = 'protocol'
  JOBINFO_FORMAT = 'jobinfo'

  def __str__(self):
    return self.value

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
  _all_probs = {} # hold info how many confs solved a problem
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
    self._total_time = 0.0
    self._total_uniqness = None
    Configuration._all_confs += 1

  def add_solved_prob(self, prob, time):
    self._num_solved += 1
    self._total_time += time
    self._probs[prob] = time
    if prob not in Configuration._all_probs:
      Configuration._all_probs[prob] = 1

  def evaluate_for_probs(self, probs, num_other_confs=None):
    if num_other_confs is None:
      num_other_confs = Configuration._all_confs

    solved, uniqness_points, time = (0, 0, 0.0)
    for prob in probs:
      if prob in self._probs:
        solved += 1
        uniqness_points += num_other_confs - Configuration._all_probs[prob]
        time += self._probs[prob]
    return (solved, uniqness_points, time)

  #in principle, num_other_confs only needs to be supplied
  #when Configuration objects are created and deleted
  #if you once parse the Conf and store it for the duration
  #of the program _all_confs will correctly count the number
  #of other configurations without you doing anything
  def evaluate_category(self, category, num_other_confs=None):
    if category in self._memo_eval:
      return self._memo_eval[category]   
    
    eval = self.evaluate_for_probs(category.get_problems(), num_other_confs)
    self._memo_eval[category] = eval
    category.store_evaluation(eval, self)
    return eval

  def as_order_key(self, num_other_confs=None):
    if self._total_uniqness is None:
      if num_other_confs is None:
        num_other_confs = Configuration._all_confs
      self._total_uniqness = 0
      for prob in self._probs.keys():
        self._total_uniqness += num_other_confs - Configuration._all_probs[prob]

    return (self._num_solved, self._total_uniqness,  -self._total_time)

  def parse_json(self, path):
    with open(path, 'r') as fd:
      import re
      self._json = re.sub(' +', ' ', fd.read()) # making the representation more compact

  def to_json(self, json_mode=BOTH):
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
