/*-----------------------------------------------------------------------

File  : ccl_factor.h

Author: Stephan Schulz

Contents

  Functions for ordered factorisation.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun May 31 19:12:41 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCL_FACTOR

#define CCL_FACTOR


#include <ccl_clausesets.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


Clause_p ComputeOrderedFactor(TB_p bank, OCB_p ocb, ClausePos_p pos1,
               ClausePos_p pos2, VarBank_p freshvars);

Eqn_p    ClausePosFirstOrderedFactorLiterals(Clause_p clause,
                    ClausePos_p pos1,
                    ClausePos_p pos2);

Eqn_p    ClausePosNextOrderedFactorLiterals(ClausePos_p pos1,
                   ClausePos_p pos2);

void     ComputeEqualityFactor(TB_p bank, OCB_p ocb, ClausePos_p pos1,
                               ClausePos_p pos2, VarBank_p freshvars,
                               bool* is_ho, PStack_p res);

Eqn_p    ClausePosFirstEqualityFactorSides(Clause_p clause,
                  ClausePos_p pos1,
                  ClausePos_p pos2);

Eqn_p    ClausePosNextEqualityFactorSides(ClausePos_p pos1,
                ClausePos_p pos2);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





