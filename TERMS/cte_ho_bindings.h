/*-----------------------------------------------------------------------

File  : cte_ho_bindings.h

Author: Petar Vukmirovic

Contents

  Interface to the module which creates higher-order variable
  bindings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> ma 25 okt 2021 10:35:21 CEST
    New

-----------------------------------------------------------------------*/

#ifndef CTE_HO_BINDINGS
#define CTE_HO_BINDINGS

#include <cte_ho_csu.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/
StateTag_t ComputeNextBinding(Term_p var, Term_p rhs, 
                                   StateTag_t state, Limits_t* limits,
                                   TB_p bank, Subst_p subst,
                                   HeuristicParms_p parms, bool* succ);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
