/*-----------------------------------------------------------------------

File  : cte_ho_csu.h

Author: Petar Vukmirovic

Contents

  Interface to algorithm for enumerating (potentially) infinite complete
  set of unifiers.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> do 21 okt 2021 13:40:13 CEST
    New

-----------------------------------------------------------------------*/


#include <cte_termtypes.h>
#include <cte_subst.h>
#include <cte_pattern_match_mgu.h>

#ifndef CTE_FULL_UNIF
#define CTE_FULL_UNIF

typedef struct csu_iter CSUIterator_t;
typedef CSUIterator_t* CSUIterator_p;

CSUIterator_p CSUIterInit(Term_p lhs, Term_p rhs, Subst_p subst);
Subst_p CSUIterGetCurrentSubst(CSUIterator_p iter);
bool NextCSUElement(CSUIterator_p iter);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
