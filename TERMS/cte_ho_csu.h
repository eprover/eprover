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
#include <che_hcb.h>

#ifndef CTE_FULL_UNIF
#define CTE_FULL_UNIF

typedef struct csu_iter CSUIterator_t;
typedef CSUIterator_t* CSUIterator_p;

#define CONSTRAINT_STATE(c) ((c)&3)
#define CONSTRAINT_COUNTER(c) ((c) >> 2) // c must be unisigned!!!
#define BUILD_CONSTR(c, s) (((c) << 2) | s)

// datatype that holds information if the rigid pair was processed
// or how far we are in the generation of binders for a flex-* pair
typedef unsigned long StateTag_t;
// datatype that encodes the limits for 1) non-simple projections
// 2) rigid imitations 3) identifiations 4) eliminations
typedef unsigned long Limits_t;

extern const StateTag_t DECOMPOSED_VAR;

CSUIterator_p CSUIterInit(Term_p lhs, Term_p rhs, Subst_p subst, TB_p bank);
void CSUIterDestroy(CSUIterator_p iter);
Subst_p CSUIterGetCurrentSubst(CSUIterator_p iter);
bool NextCSUElement(CSUIterator_p iter);
void InitUnifLimits(HeuristicParms_p p);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
