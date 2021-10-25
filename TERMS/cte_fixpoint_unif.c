/*-----------------------------------------------------------------------

File  : cte_pattern_match_mgu.c

Author: Stephan Schulz

Contents

  Implementation of simple, non-indexed 1-1 match and unification
  routines on shared terms (and unshared terms with shared
  variables).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Mar 11 16:17:33 MET 1998
    New

-----------------------------------------------------------------------*/

#include "cte_pattern_match_mgu.h"
#include <cte_lambda.h>
#include <clb_plocalstacks.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

inline OracleUnifResult rigid_path_check_args(Term_p t1, Term_p* args, 
                                              bool under_var, bool has_pref,
                                              int depth, int length);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: rigid_path_check()
//
//   Computes the fixpoint unifier of two terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult rigid_path_check(Term_p t1, Term_p t2, bool has_pref, 
                                  bool under_var, int depth)
{
   OracleUnifResult res;
   if(TermIsAppliedFreeVar(t2))
   {
      if(t1 == t2->args[0])
      {
         res = (under_var || has_pref) ? NOT_IN_FRAGMENT : NOT_UNIFIABLE;
      }
      else
      {
         res = rigid_path_check_args(t1, t2->args+1, t2->arity-1, 
                                     has_pref, true, depth);
      }
   }
   else if(TermIsFreeVar(t2))
   {
      res = (under_var || TypeIsArrow(t2->type)) ? NOT_IN_FRAGMENT : NOT_UNIFIABLE;
   }
   else if(TermIsLambda(t2))
   {
      while(TermIsLambda(t2))
      {
         t2 = t2->args[1];
         depth++;
      }
      res = rigid_path_check(t1, t2, has_pref, under_var, depth);
   }
   else if(TermIsDBVar(t2))
   {
      if(t2->f_code >= depth)
      {
         res = under_var ? NOT_IN_FRAGMENT : NOT_UNIFIABLE;
      }
      else
      {
         res = UNIFIABLE;
      }
   }
   else
   {
      res = rigid_path_check_args(t1, t2->args, t2->arity,
                                  has_pref, under_var, depth);
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: rigid_path_check_args()
//
//   Does the same as rigid_path_check but for an array of terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

inline OracleUnifResult rigid_path_check_args(Term_p t1, Term_p* args, bool has_pref, 
                                              bool under_var, int depth, int length)
{
   OracleUnifResult res = UNIFIABLE;
   for(int i=0; res == UNIFIABLE && i<length; i++)
   {
      res = rigid_path_check(t1, args[i], has_pref, under_var, depth);
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: SubstComputeFixpointMgu()
//
//   Computes the fixpoint unifier of two terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

OracleUnifResult SubstComputeFixpointMgu(Term_p t1, Term_p t2, Subst_p subst)
{
   t1 = LambdaEtaReduceDB(TermGetBank(t1), WHNF_deref(t1));
   t2 = LambdaEtaReduceDB(TermGetBank(t2), WHNF_deref(t2));

   OracleUnifResult res;
   if(TermIsFreeVar(t1) && TermIsFreeVar(t2))
   {
      if(t1 != t2)
      {
         SubstAddBinding(subst, t1, t2);
      }
      res = UNIFIABLE;
   }
   else if(!TermIsFreeVar(t1) && !TermIsFreeVar(t2))
   {
      res = NOT_IN_FRAGMENT;
   }
   else
   {
      if(!TermIsFreeVar(t1))
      {
         SWAP(t1, t2);
      }
      assert(TermIsFreeVar(t1));
      assert(!TermIsFreeVar(t2));

      res = rigid_path_check(t1, t2, TermIsLambda(t2), false, 0);
      if(res == UNIFIABLE)
      {
         SubstAddBinding(subst, t1, t2);
      }
   }

   return res;
}
