/*-----------------------------------------------------------------------

File  : ccl_tautologies.h

Author: Stephan Schulz

Contents

  Functions for detecting tautologies using the algorithm suggested by
  Roberto Nieuwenhuis: Do ground completion on negative literals, see
  if they imply the positive ones

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue May  4 17:23:56 MEST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CCL_TAUTOLOGIES

#define CCL_TAUTOLOGIES

#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MAX_EQ_TAUTOLOGY_CHECK_LITNO 1000

bool ClauseIsTautologyReal(TB_p work_bank, Clause_p clause, bool copy);

#define ClauseIsTautology(b,c) (ClauseIsTautologyReal(b, c, true))

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
