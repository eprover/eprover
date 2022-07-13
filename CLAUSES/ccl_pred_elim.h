/*-----------------------------------------------------------------------

File  : clb_pred_elim.h

Author: Petar Vukmirovic

Contents

  Implements (defined) predicate elimination as described in 
  SAT-inspired eliminations for superposition
  (https://ieeexplore.ieee.org/document/9617710).

Copyright 1998-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> vr  7 jan 2022 13:34:39 CET
-----------------------------------------------------------------------*/

#ifndef CCL_PE
#define CCL_PE

#include <ccl_clausesets.h>
#include <che_hcb.h>

void PredicateElimination(ClauseSet_p passive, ClauseSet_p archive,
                          const HeuristicParms_p parms,  TB_p bank,
                          TB_p tmp_bank, VarBank_p fresh_vars);

#endif
