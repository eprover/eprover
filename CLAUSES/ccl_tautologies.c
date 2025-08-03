/*-----------------------------------------------------------------------

  File  : ccl_tautologies.c

  Author: Stephan Schulz

  Contents

  Functions for detecting tautologies using the algorithm suggested by
  Roberto Nieuwenhuis: Do ground completion on negative literals, see
  if they imply the positive ones

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created:  Tue May  4 17:41:06 MEST 1999

  -----------------------------------------------------------------------*/

#include "ccl_tautologies.h"
#include "ccl_derivation.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TO_ground_compare()
//
//   Compare two terms with a very simple total ordering extendable to
//   a reduction ordering.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CompareResult TO_ground_compare(Term_p t1, Term_p t2)
{
   long          w1, w2;
   CompareResult res = to_equal;
   int           i;
   PStack_p      stack = PStackAlloc();

   PStackPushP(stack, t1);
   PStackPushP(stack, t2);

   while(!PStackEmpty(stack))
   {
      t2 = PStackPopP(stack);
      t1 = PStackPopP(stack);

      w1 = TermStandardWeight(t1);
      w2 = TermStandardWeight(t2);

      if(w1<w2)
      {
         res = to_lesser;
      }
      else if(w1>w2)
      {
         res = to_greater;
      }
      else if(t1->f_code < t2->f_code)
      {
         res = to_lesser;
      }
      else if(t1->f_code > t2->f_code)
      {
         res = to_greater;
      }
      else if(t1->arity < t2->arity)
      {
         res = to_lesser;
      }
      else if(t1->arity > t2->arity)
      {
         res = to_greater;
      }
      if(res!= to_equal)
      {
         break;
      }
      assert(problemType == PROBLEM_HO || t1->arity == t2->arity);
      for(i = 0; i<t1->arity; i++)
      {
         PStackPushP(stack, t1->args[i]);
         PStackPushP(stack, t2->args[i]);
      }
   }
   PStackFree(stack);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ground_orient_eqn()
//
//   Orient an equation (by setting or deleting appropriate
//   flag). Return true if terms are different, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Sets flag
//
/----------------------------------------------------------------------*/

static bool ground_orient_eqn(Eqn_p eqn)
{
   CompareResult cmp;

   cmp = TO_ground_compare(eqn->lterm, eqn->rterm);
   assert(cmp != to_uncomparable);

   if(cmp == to_greater)
   {
      EqnSetProp(eqn, EPGONatural);
   }
   else
   {
      EqnDelProp(eqn, EPGONatural);
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: term_compute_top_nf()
//
//   Checks if one of the eqns can reduce *ref, if yes
//   does so and returns true. Otherwise returns false.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static bool term_compute_top_nf(TermRef ref, Eqn_p eqns)
{
   Eqn_p handle = eqns;
   Term_p lside, rside;

   while(handle)
   {
      if(EqnQueryProp(handle, EPGONatural))
      {
         lside = handle->lterm;
         rside = handle->rterm;
      }
      else
      {
         lside = handle->rterm;
         rside = handle->lterm;
      }
      if(TermStructEqualNoDeref(lside, *ref))
      {
         TermFree(*ref);
         *ref = TermCopy(rside, handle->bank->vars, handle->bank->db_vars, false);
         return true;
      }
      handle = handle->next;
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: term_compute_ground_NF()
//
//   Compute a ground normal form of *ref with respect to eqns. *ref
//   should be unshared, eqns should be interreduced. Return true if
//   term changed. This is probably
//   not an optimal implementation, but *ref and eqns should be pretty
//   small and not worth any of the overhead of the more sophisticated
//   algorithms.
//
// Global Variables: -
//
// Side Effects    : Memory management, changes *ref
//
/----------------------------------------------------------------------*/

static bool term_compute_ground_NF(TermRef ref, Eqn_p eqns)
{
   int i;
   bool res = false, tmp;

   for(i=0 ; i< (*ref)->arity; i++)
   {
      tmp = term_compute_ground_NF((&(*ref)->args[i]), eqns);
      res = res||tmp;
   }
   tmp = term_compute_top_nf(ref, eqns);
   res = res||tmp;

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ground_normalize_eqn()
//
//   Normalize eqn with respect to eqns (which should be
//   interreduced). Return true if maximal side has been rewritten,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static bool ground_normalize_eqn(Eqn_p eqn, Eqn_p eqns)
{
   bool res = false, tmp;
   Term_p term, shared;

   term = TermCopy(eqn->lterm, eqn->bank->vars, eqn->bank->db_vars, DEREF_NEVER);
   tmp  = term_compute_ground_NF(&term, eqns);
   if(tmp)
   {
      shared = TBInsert(eqn->bank, term, DEREF_NEVER);
      eqn->lterm = shared;
      res = EqnQueryProp(eqn, EPGONatural);
   }
   TermFree(term);

   term = TermCopy(eqn->rterm, eqn->bank->vars, eqn->bank->db_vars, DEREF_NEVER);
   tmp  = term_compute_ground_NF(&term, eqns);
   if(tmp)
   {
      shared = TBInsert(eqn->bank, term, DEREF_NEVER);
      eqn->rterm = shared;
      res = res || EqnQueryProp(eqn, EPGONatural);
   }
   TermFree(term);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ground_backward_contract()
//
//   Normalize all eqations in from with respect to eqns. Put those
//   whose maximal side has changed into to.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static void ground_backward_contract(EqnRef from, Eqn_p eqns, EqnRef to)
{
   Eqn_p handle;
   bool   res;

   while(*from)
   {
      res = ground_normalize_eqn(*from, eqns);
      if(res)
      {
         handle = EqnListExtractFirst(from);
         EqnListInsertFirst(to, handle);
      }
      else
      {
         from = &((*from)->next);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ground_complete_neg_eqns()
//
//   Complete the negative equations in *list. Return completed
//   system.
//
// Global Variables: -
//
// Side Effects    : Changes *list, memory operations
//
/----------------------------------------------------------------------*/

static void ground_complete_neg_eqns(EqnRef list)
{
   Eqn_p unproc = NULL, proc = NULL, handle;
   bool  cmp;

   unproc = *list;

   while(unproc)
   {
      handle = EqnListExtractFirst(&unproc);
      assert(!handle->next);
      ground_normalize_eqn(handle, proc);
      if(handle->lterm==handle->rterm)
      {
         EqnFree(handle);
         continue;
      }
      cmp = ground_orient_eqn(handle);
      UNUSED(cmp); assert(cmp);
      ground_backward_contract(&proc, handle, &unproc);
      EqnListInsertFirst(&proc, handle);
   }
   *list = proc;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseIsTautologyReal()
//
//   Return true if clause certainly is a tautology, false if this
//   cannot be shown at the accepted expense.
//
// Global Variables: -
//
// Side Effects    : Possibly memory operations
//
//----------------------------------------------------------------------*/

bool ClauseIsTautologyReal(TB_p work_bank, Clause_p clause, bool copy_cl)
{
   Eqn_p    rw_system, handle;
   Clause_p work_copy;
   bool     res = false;

   if(EqnListFindTrue(clause->literals))
   {
      return true;
   }
   if((clause->pos_lit_no==0) || (clause->neg_lit_no==0))
   {
      return false;
   }
   if(clause->neg_lit_no > MAX_EQ_TAUTOLOGY_CHECK_LITNO)
   { /* Emergency exit for large clauses! */
      //printf(COMCHAR" ClauseIsTautology() - neg_lit_no: %d\n", clause->neg_lit_no);
      return ClauseIsTrivial(clause);
   }
   work_copy = copy_cl ? ClauseCopy(clause, work_bank) : clause;
   rw_system = EqnListExtractByProps(&(work_copy->literals),
                                     EPIsPositive, true);
   assert(rw_system);
   if(clause->neg_lit_no > 1)
   {
      ground_complete_neg_eqns(&rw_system);
   }
   else
   {
      ground_orient_eqn(rw_system);
   }

   for(handle = work_copy->literals; handle; handle = handle->next)
   {
      assert(EqnIsPositive(handle));

      ground_normalize_eqn(handle, rw_system);
      if(handle->lterm == handle->rterm)
      {
         res = true;
         break;
      }
   }
   EqnListFree(rw_system);
   ClauseFree(work_copy);

   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
