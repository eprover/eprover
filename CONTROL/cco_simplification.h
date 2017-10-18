/*-----------------------------------------------------------------------

File  : cco_simplification.h

Author: Stephan Schulz

Contents

  Global control function used with simplification.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 14:49:49 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CCO_SIMPLIFICATION

#define CCO_SIMPLIFICATION


#include <che_proofcontrol.h>
#include <ccl_rewrite.h>
#include <ccl_context_sr.h>
#include <ccl_global_indices.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void     ClauseMoveSimplified(GlobalIndices_p gindices,
                              Clause_p clause,
                              ClauseSet_p tmp_set,
                              ClauseSet_p archive);

bool     RemoveRewritableClauses(OCB_p ocb, ClauseSet_p from,
             ClauseSet_p into,
                                 ClauseSet_p archive,
                                 Clause_p new_demod, SysDate nf_date,
                                 GlobalIndices_p gindices);
bool     RemoveRewritableClausesIndexed(OCB_p ocb, ClauseSet_p into,
                                        ClauseSet_p archive,
                                        Clause_p new_demod, SysDate nf_date,
                                        GlobalIndices_p gindices);

long ClauseSetUnitSimplify(ClauseSet_p set, Clause_p simplifier,
            ClauseSet_p tmp_set, ClauseSet_p archive,
                           GlobalIndices_p gindices);

long     RemoveContextualSRClauses(ClauseSet_p from,
               ClauseSet_p into,
                                   ClauseSet_p archive,
                                   Clause_p simplifier,
                                   GlobalIndices_p gindices);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/












