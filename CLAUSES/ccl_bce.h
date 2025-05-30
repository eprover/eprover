/*-----------------------------------------------------------------------

  File  : clb_bce.h

  Author: Petar Vukmirovic

  Contents

  Implements blocked clause elimination as described in
  Blocked Clauses in First-Order Logic (https://doi.org/10.29007/c3wq).

  Copyright 1998-2022 by the author.

  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Di 4 jan 2022 13:00:10 CET

-----------------------------------------------------------------------*/

#ifndef CCL_BCE
#define CCL_BCE

#include <ccl_clausesets.h>

void EliminateBlockedClauses(ClauseSet_p set, ClauseSet_p archive,
                             int max_occs, TB_p tmp_bank);

#endif
