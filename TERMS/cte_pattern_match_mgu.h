/*-----------------------------------------------------------------------

File  : cte_pattern_match_mgu.h

Author: Petar Vukmirovic

Contents

  Interface to simple, non-indexed 1-1 match and unification
  routines on shared *higher-order pattern* terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> di 20 jul 2021  9:06:46 UTC
    New

-----------------------------------------------------------------------*/

#ifndef CTE_PATTERN_UNIF
#define CTE_PATTERN_UNIF

#include <cte_termtypes.h>
#include <cte_subst.h>
#include <cte_match_mgu_1-1.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
Term_p FreshVarWArgs(TB_p bank, PStack_p args, Type_p ret_ty);
bool PruneLambdaPrefix(TB_p bank, Term_p *t1_ref, Term_p *t2_ref);
OracleUnifResult SubstComputeMguPattern(Term_p t1, Term_p t2, Subst_p subst);
OracleUnifResult SubstComputeMatchPattern(Term_p matcher, Term_p to_match, Subst_p subst);

/*---------------------------------------------------------------------*/
/*                Inline Functions                                     */
/*---------------------------------------------------------------------*/

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
