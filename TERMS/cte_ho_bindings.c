/*-----------------------------------------------------------------------

File  : cte_ho_bindings.c

Author: Petar Vukmirovic.

Contents

  Implementation of the module which creates higher-order variable
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

#include "cte_ho_bindings.h"

#define CONSTRAINT_STATE (c) ((c) & 3)
#define CONSTRAINT_COUNTER (c) ((c) >> 2) // c must be unisigned!!!

#define IMIT_MASK (63U)
#define PROJ_MASK (IMIT_MASK << 6)
#define IDENT_MASK (PROJ_MASK << 6)
#define ELIM_MASK (IDENT_MASK << 6)


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: SubstComputeFixpointMgu()
//
//   Assuming that var is an (applied) variable and rhs an arbitrary term
//   which are normalized and to which substitution is applied generate
//   the next binding in an attempt to solve the problem var =?= rhs. 
//   What the next binding is is determined by the value of 'state'.
//   The last two bits of 'state' have special meaning (is the variable
//   pair already processed) and the remaining bits determine how far
//   in the enumeration of bindings we are. 'applied_bs' counts how
//   many bindings of a certain kind are applied. It is a value that
//   is inspected through bit masks that give value of particular bindings.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

ConstraintTag_t ComputeNextBinding(Term_p var, Term_p rhs, 
                                   ConstraintTag_t state, Limits_t applied_bs,
                                   TB_p bank, Subst_p subst,
                                   HeuristicParms_p parms)
{
   assert(TermIsTopLevelFreeVar(var));
   return state;
}
