/*-----------------------------------------------------------------------

File  : cco_simplification.h

Author: Stephan Schulz

Contents
 
  Global control function used with simplification.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 14:49:49 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCO_SIMPLIFICATION

#define CCI_SIMPIFICATION


#include <che_proofcontrol.h>
#include <ccl_rewrite.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void     DemodInsert(ClauseSet_p set, Clause_p new);
void     ClauseMoveSimplified(Clause_p clause, ClauseSet_p tmp_set);
bool     RemoveClausesWithRewritableMaxSides(OCB_p ocb, ClauseSet_p from,
					     ClauseSet_p into, Clause_p
					     new_demod, SysDate nf_date);
bool     RemoveRewritableClauses(OCB_p ocb, ClauseSet_p from,
				 ClauseSet_p into, Clause_p
				 new_demod, SysDate nf_date);
long     ClauseSetUnitSimplify(ClauseSet_p set, Clause_p simplifier,
			       ClauseSet_p tmp_set);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/












