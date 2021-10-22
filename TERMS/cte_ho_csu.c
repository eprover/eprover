/*-----------------------------------------------------------------------

File  : cte_ho_csu.c

Author: Stephan Schulz

Contents

  Implementation of algorithm for enumerating (potentially) infinite complete
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

#include "cte_ho_csu.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

// datatype that holds information if the rigid pair was processed
// or how far we are in the generation of binders for a flex-* pair
typedef unsigned long ConstraintTag_t;

ConstraintTag_t INIT_TAG = 0;
ConstraintTag_t RIGID_PROCESSED_TAG = 1;
ConstraintTag_t SOLVED_BY_ORACLE_TAG = 2;

struct csu_iter 
{
   PStack_p constraints; // pairs of terms to unify
   // quadruples (LHS, RHS, how was the constraint solved, subst before solution)
   PStack_p backtrack_info;
   // in what state is the current solving of the pair?
   ConstraintTag_t current_state;
   PStackPointer init_pos;
   Subst_p subst;
   TB_p bank;
};

#define GET_HEAD_ID(t) (TermIsPhonyApp(t) ? (t)->args[0]->f_code : (t)->f_code)
#define CSUIterAlloc() (SizeMalloc(sizeof(struct csu_iter)))

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
// Function: destroy_iter()
//
//  Destroys the iter and frees all the memory EXCEPT for the initial
//  substitution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void destroy_iter(CSUIterator_p iter)
{
   PStackFree(iter->backtrack_info);
   PStackFree(iter->constraints);
   SubstBacktrackToPos(iter->subst, iter->init_pos);
   SizeFree(iter, sizeof(CSUIterator_t));
}

/*-----------------------------------------------------------------------
//
// Function: store_backtrack_pair()
//
//   Prepare the backtracking state.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void store_backtrack_pair(CSUIterator_p iter, Term_p lhs, Term_p rhs,
                          ConstraintTag_t new_tag)
{
   PStackPushInt(iter->backtrack_info, PStackGetSP(iter->subst));
   PStackPushInt(iter->backtrack_info, new_tag);
   PStackPushP(iter->backtrack_info, rhs);
   PStackPushP(iter->backtrack_info, lhs);
}

/*-----------------------------------------------------------------------
//
// Function: forward_iter()
//
//   After the iterator has successfully been backtracked, 
//   try to find the solution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool forward_iter(CSUIterator_p iter)
{
   bool res = true;
   while(res && !PStackEmpty(iter->constraints))
   {
      assert(PStackGetSP(iter->constraints) % 2 == 0);
      Term_p lhs = WHNF_deref(PStackPopP(iter->constraints));
      Term_p rhs = WHNF_deref(PStackPopP(iter->constraints));
      PruneLambdaPrefix(iter->bank, &lhs, &rhs);

      if(lhs->type != rhs->type)
      {
         assert(iter->current_state == INIT_TAG);
         res = false;
      }
      else if(lhs == rhs)
      {
         store_backtrack_pair(iter, lhs, rhs, SOLVED_BY_ORACLE_TAG);
      }
      else
      {
         if(TermIsTopLevelFreeVar(rhs))
         {
            SWAP(lhs, rhs);
         }

         // if either of the sides is app var
         // LHS must be free var
         if(TermIsTopLevelFreeVar(lhs))
         {
            // TODO:
            // try oracles
            // if oracles do not work,
            // try getting the binder
         }
         else
         {
            if(TermIsPhonyApp(lhs))
            {
               if(TermIsPhonyApp(rhs))
               {
                  assert(TermIsDBVar(lhs->args[0]));
                  assert(TermIsDBVar(rhs->args[0]));
                  if(lhs->args[0] == rhs->args[0])
                  {
                     assert(lhs->arity == rhs->arity);
                     schedule_args(iter, lhs->args+1, rhs->args+1, lhs->arity-1);
                     store_backtrack_pair(iter, lhs, rhs, RIGID_PROCESSED_TAG);
                  }
                  else
                  {
                     res = backtrack_iter(iter);
                  }
               }
               else
               {
                  res = backtrack_iter(iter);
               }
            }
            else if(TermIsDBVar(lhs))
            {
               if(!TermIsDBVar(rhs) || lhs->f_code != rhs->f_code)
               {
                  res = backtrack_iter(iter);
               }
               else
               {
                  store_backtrack_pair(iter, lhs, rhs, RIGID_PROCESSED_TAG);
               }
            }
            else if (lhs->f_code == rhs->f_code)
            {
               assert(lhs->arity == rhs->arity);
               if(SigIsPolymorphic(iter->bank->sig, lhs->f_code)
                  && lhs->arity != 0
                  && lhs->args[0]->type != rhs->args[0]->type) 
               {
                  res = backtrack_iter(iter);
               }
               else
               {
                  schedule_jobs(jobs, t1->args, t2->args, t1->arity);
               }
            }
         }

      }


   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: backtrack_iter()
//
//   After the call to CSUIterInit or successful call to NextCSUElement,
//   set the state of the iterator so that it is ready to advance to 
//   the next iterator. If false is returned, there are no more solutions
//   and the iterator shall be destroyed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool backtrack_iter(CSUIterator_p iter)
{
   bool res = false;
   if(!PStackEmpty(iter->backtrack_info))
   {
      assert(PStackGetSP(iter->backtrack_info) >= 4);
      while(PStackGetSP(iter->backtrack_info) >= 4 && !res)
      {
         assert(PStackGetSP(iter->backtrack_info) % 4 == 0);
         Term_p lhs = PStackPopP(iter->backtrack_info);
         Term_p rhs = PStackPopP(iter->backtrack_info);
         ConstraintTag_t constr_tag = PStackPopInt(iter->backtrack_info);
         PStackPointer subst_pointer = PStackPopInt(iter->backtrack_info);

         if(constr_tag == INIT_TAG)
         {
            iter->current_state = INIT_TAG;
            PStackPushP(iter->constraints, rhs);
            PStackPushP(iter->constraints, lhs);
            res = true;
         }
         else
         {
            int to_drop;
            if(constr_tag == SOLVED_BY_ORACLE_TAG)
            {
               to_drop = 0;
            }
            else if(TermIsTopLevelFreeVar(lhs) || TermIsTopLevelFreeVar(rhs))
            {
               // variables are somehow resolved, and
               // thus we remove the result of this resolving
               // to backtrack
               to_drop = 1;
               iter->current_state = constr_tag;
               res = true; // we backtrack to first variable
            }
            else
            {
               assert(constr_tag == RIGID_PROCESSED_TAG);
               assert(TermIsPhonyApp(lhs) == TermIsPhonyApp(rhs));
               assert(GET_HEAD_ID(lhs) == GET_HEAD_ID(rhs));
               assert(lhs->arity == rhs->arity);

               to_drop = lhs->arity - (TermIsPhonyApp(lhs) ? 1 : 0);
            }
            
            for(int i=0; i<to_drop; i++)
            {
               PStackPop(iter->constraints);
               PStackPop(iter->constraints);
            }
            PStackPushP(iter->constraints, rhs);
            PStackPushP(iter->constraints, lhs);
            SubstBacktrackToPos(iter->subst, subst_pointer);
         }
      }
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: NextCSUElement()
//
//   Given a (previously initialized) iterator if there exists a next
//   unifier return true and set the substitution of the iterator 
//   to the unifier. If there is no unifier, all the variables are
//   unbound and false is returned. When false is returned, CSUIterator
//   is destroyed an is no longer to be used.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool NextCSUElement(CSUIterator_p iter)
{
   bool res = false;
   if(backtrack_iter(iter))
   {
      res = forward_iter(iter);
   }
   if(!res)
   {
      destroy_iter(iter);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: CSUIterInit()
//
//   Given a (previously initialized) iterator if there exists a next
//   unifier return true and set the substitution of the iterator 
//   to the unifier. If there is no unifier, all the variables are
//   unbound and false is returned. When false is returned, CSUIterator
//   is destroyed an is no longer to be used.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

CSUIterator_p CSUIterInit(Term_p lhs, Term_p rhs, Subst_p subst, TB_p bank)
{
   CSUIterator_p res = CSUIterAlloc();
   res->subst = subst;
   res->init_pos = PStackGetSP(subst);
   res->backtrack_info = PStackAlloc();
   res->constraints = PStackAlloc();
   res->bank = bank;

   // initialization
   PStackPushInt(res->backtrack_info, res->init_pos);
   PStackPushInt(res->backtrack_info, INIT_TAG);
   PStackPushP(res->backtrack_info, rhs);
   PStackPushP(res->backtrack_info, lhs);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: CSUIterGetCurrentSubst()
//
//   Returns the substitution stored in the iterator. NB: User needs
//   to take care that substitution is only observed in the correct
//   states.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Subst_p CSUIterGetCurrentSubst(CSUIterator_p iter)
{
   return iter->subst;
}
