/*-----------------------------------------------------------------------

File  : cte_fixpoint_unif.h

Author: Petar Vukmirovic

Contents

  Interface to fixpoint decider.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> ma 25 okt 2021 10:35:21 CEST
    New

-----------------------------------------------------------------------*/

#ifndef CTE_FIXPOINT_UNIF
#define CTE_FIXPOINT_UNIF

#include <cte_termtypes.h>
#include <cte_subst.h>
#include <cte_match_mgu_1-1.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
OracleUnifResult SubstComputeFixpointMgu(Term_p t1, Term_p t2, Subst_p subst);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
