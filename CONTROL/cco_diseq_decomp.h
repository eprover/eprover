/*-----------------------------------------------------------------------

  File  : cco_diseq_decomp.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Code to control the computation of disequality decomposition. The
  disequality decomposition inference is

  f(s1,...,sn)!=f(t1,...,tn) | R
  -------------------------------
  s1!=t1 | ... | sn_tn | R

  Copyright 2024 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Tue Mar 12 17:50:09 CET 2024

-----------------------------------------------------------------------*/

#ifndef CCO_DISEQ_DECOMP

#define CCO_DISEQ_DECOMP

#include <ccl_clausesets.h>
#include <ccl_diseq_decomp.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


long ComputeDisEqDecompositions(TB_p terms, Clause_p clause,
                                ClauseSet_p store,
                                long diseq_decomposition,
                                long diseq_decomp_maxarity);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
