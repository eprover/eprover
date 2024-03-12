/*-----------------------------------------------------------------------

  File  : ccl_diseq_decomp.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Implement the disequality decomposition inference.

  f(s1,...,sn)!=f(t1,...,tn) | R
  -------------------------------
  s1!=t1 | ... | sn_tn | R

  This wraps the actual inference into one small function.

  Copyright 2024 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:

-----------------------------------------------------------------------*/

#ifndef CCL_DISEQ_DECOMP

#define CCL_DISEQ_DECOMP

#include <ccl_clauses.h>
#include <ccl_clausecpos.h>
#include <ccl_derivation.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


Clause_p ClauseDisEqDecomposition(TB_p terms, Clause_p clause, CompactPos litpos);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
