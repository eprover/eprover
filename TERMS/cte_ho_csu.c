/*-----------------------------------------------------------------------

File  : cte_ho_csu.c

Author: Petar Vukmirovic

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
#include "cte_ho_bindings.h"
#include <cte_lambda.h>
#include <che_hcb.h>
#include <stdint.h>
#include <cte_fixpoint_unif.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

const StateTag_t INIT_TAG = 0;
const StateTag_t RIGID_PROCESSED_TAG = 1;
const StateTag_t SOLVED_BY_ORACLE_TAG = 2;
const StateTag_t DECOMPOSED_VAR = 3;

#define BT_STEP_SIZE 4

struct csu_iter
{
   PQueue_p constraints; // pairs of terms to unify
   // quadruples (old constraints, state, limits, pointer)
   PStack_p backtrack_info;
   // in what state is the current solving of the pair?
   StateTag_t current_state;
   // what are the current limit bindings
   Limits_t current_limits;
   PStackPointer init_pos;
   Subst_p subst;
   TB_p bank;
   int unifiers_returned;
   // steps that are done in one call to NextIter()
   int steps;

   // implementation-specific data used for optimization
   PStack_p tmp_rigid_diff;
   PStack_p tmp_rigid_same;
   PStack_p tmp_flex;
#ifndef NDEBUG
   Term_p orig_lhs;
   Term_p orig_rhs;
#endif
};

#define GET_HEAD_ID(t) (TermIsPhonyApp(t) ? (t)->args[0]->f_code : (t)->f_code)
#define CSUIterAlloc() (SizeMalloc(sizeof(struct csu_iter)))

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

static HeuristicParms_p params = NULL;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

bool backtrack_iter(CSUIterator_p iter);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: dbg_print_state()
//
//   Print the state in human-readeable format.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void dbg_print_state(FILE* out, CSUIterator_p iter)
{
   TB_p bank = iter->bank;
   // fprintf(out, "***state(%ld):\n", iter->current_state);
   fprintf(out, "[");
   if(!PQueueEmpty(iter->constraints))
   {
      PQueue_p q = iter->constraints;
      fprintf(stderr, "%ld", PQueueCardinality(q));
   }
   fprintf(out, "]\nbt_size:%ld\n", PStackGetSP(iter->backtrack_info));
   SubstPrint(out, iter->subst, bank->sig, DEREF_NEVER);
   fprintf(out, "\n");
}

/*-----------------------------------------------------------------------
//
// Function: whnf_and_prune()
//
//   Normalize heads and remove the possible lambda prefixes.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void whnf_and_prune(TB_p bank, Term_p* l, Term_p* r)
{
   *l = WHNF_deref(*l);
   *r = WHNF_deref(*r);
   PruneLambdaPrefix(bank, l, r);
}

/*-----------------------------------------------------------------------
//
// Function: build_new_queue()
//
//   Builds a copy of the queue that is to be used for backtracking.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

PQueue_p build_new_queue(PQueue_p old, Term_p lhs, Term_p rhs)
{
   PQueue_p res = PQueueCellAlloc();
   *res = *old; // copy all fields
   res->queue = SizeMalloc(old->size*sizeof(IntOrP));
   memcpy(res->queue, old->queue, old->size*sizeof(IntOrP));

   PQueueStoreP(res, rhs);
   PQueueStoreP(res, lhs);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: prepare_backtrack()
//
//   Prepare the backtracking state.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void prepare_backtrack(CSUIterator_p iter, Term_p lhs, Term_p rhs, StateTag_t next_state,
                       Limits_t next_limits, PStackPointer subst_ptr)
{
   PStackPushP(iter->backtrack_info, build_new_queue(iter->constraints, lhs, rhs));
   PStackPushInt(iter->backtrack_info, next_state);
   PStackPushInt(iter->backtrack_info, next_limits);
   PStackPushInt(iter->backtrack_info, subst_ptr);
}

/*-----------------------------------------------------------------------
//
// Function: unroll_fcode()
//
//   Go under lamdbas and follow binding pointers until we either hit
//   rigid symbol or there are no more bindings
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode unroll_fcode(Term_p t)
{
   while(TermIsLambda(t))
   {
      t = t->args[1];
   }

   while(CAN_DEREF(t))
   {
      Term_p var = TermIsAppliedFreeVar(t) ? t->args[0] : t;
      t = var->binding;
      while(TermIsLambda(t))
      {
         t = t->args[1];
      }
   }

   return (TermIsPhonyApp(t) ? t->args[0] : t)->f_code;
}

/*-----------------------------------------------------------------------
//
// Function: move_stack()
//
//   Push one stack to the other one and reset the original one.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define BURY_KIND 0
#define STORE_KIND 1
void move_stack(PQueue_p new, PStack_p old, int move_kind)
{
   while(!PStackEmpty(old))
   {
      Term_p t = PStackPopP(old);
      assert(t);
      if (move_kind == BURY_KIND)
      {
         PQueueBuryP(new, t);
      }
      else
      {
         PQueueStoreP(new, t);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: schedule_args()
//
//   Put the arguments on the constraints stack in the order which
//   improves the performance of unif algorithm: first rigid-rigid
//   of different values, then other rigid-rigid and then flex-flex-pairs
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void schedule_args(CSUIterator_p iter, Term_p* l_args, Term_p* r_args, int size)
{
   assert(PStackEmpty(iter->tmp_flex) && PStackEmpty(iter->tmp_rigid_diff)
                                      && PStackEmpty(iter->tmp_rigid_same));
   for(int i=0; i<size; i++)
   {
      FunCode l_fc = unroll_fcode(l_args[i]), r_fc = unroll_fcode(r_args[i]);
      if(l_fc < 0 || r_fc < 0)
      {
         PStackPushP(iter->tmp_flex, r_args[i]);
         PStackPushP(iter->tmp_flex, l_args[i]);
      }
      else if(l_fc == r_fc)
      {
         PStackPushP(iter->tmp_rigid_same, r_args[i]);
         PStackPushP(iter->tmp_rigid_same, l_args[i]);
      }
      else
      {
         PStackPushP(iter->tmp_rigid_diff, r_args[i]);
         PStackPushP(iter->tmp_rigid_diff, l_args[i]);
      }
   }

   move_stack(iter->constraints, iter->tmp_rigid_same, STORE_KIND);
   move_stack(iter->constraints, iter->tmp_rigid_diff, STORE_KIND);
   move_stack(iter->constraints, iter->tmp_flex, BURY_KIND);
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
   while(res && !PQueueEmpty(iter->constraints))
   {
      assert(PQueueCardinality(iter->constraints) % 2 == 0);
      // items are poped from the stack only when they are solved
      Term_p lhs = PQueueGetLastP(iter->constraints);
      Term_p rhs = PQueueGetLastP(iter->constraints);
      assert(lhs);
      assert(rhs);

      if(params->max_unif_steps > 0 && iter->steps >= params->max_unif_steps)
      {
         res = backtrack_iter(iter);
         continue;
      }
      else if(lhs->type != rhs->type)
      {
         assert(PQueueEmpty(iter->constraints)); // only in the beginning
         res = false;
      }
      else
      {
         whnf_and_prune(iter->bank, &lhs, &rhs);
         PStackPointer subst_ptr = PStackGetSP(iter->subst);
         // DBG_PRINT(stderr, "\noriginal: ", TermPrint(stderr, iter->orig_lhs, iter->bank->sig, DEREF_NEVER), " <> ");
         // DBG_PRINT(stderr, "", TermPrint(stderr, iter->orig_rhs, iter->bank->sig, DEREF_NEVER), ".\n");
         // DBG_PRINT(stderr, "\nsolving: ", TermPrint(stderr, lhs, iter->bank->sig, DEREF_NEVER), " <> ");
         // DBG_PRINT(stderr, "", TermPrint(stderr, rhs, iter->bank->sig, DEREF_NEVER), ".\n");
         // DBG_PRINT(stderr, "subst:", SubstPrint(stderr, iter->subst, iter->bank->sig, DEREF_NEVER), ".\n");
         // dbg_print_state(stderr, iter);
         if(lhs == rhs)
         {
            continue; // using continue not to indent too much :(
         }
         else if(TermIsGround(lhs) && TermIsGround(rhs))
         {
            assert(lhs != rhs);
            res = backtrack_iter(iter);
            continue;
         }

         if(TermIsTopLevelFreeVar(rhs) && !TermIsTopLevelFreeVar(lhs))
         {
            SWAP(lhs, rhs);
         }

         if(TermIsTopLevelFreeVar(lhs))
         {
            OracleUnifResult oracle_res = NOT_IN_FRAGMENT;
            if(params->fixpoint_oracle)
            {
               oracle_res = SubstComputeFixpointMgu(lhs, rhs, iter->subst);
            }
            if(oracle_res == NOT_IN_FRAGMENT && params->pattern_oracle)
            {
               oracle_res = SubstComputeMguPattern(lhs, rhs, iter->subst);
            }

            if(oracle_res == NOT_UNIFIABLE)
            {
               res = backtrack_iter(iter);
            }
            else if (oracle_res == NOT_IN_FRAGMENT)
            {
               iter->steps++; // something is going to be applied to variable
               bool moved_forward;
               Limits_t next_limits = iter->current_limits;
               StateTag_t next_state =
                  ComputeNextBinding(lhs, rhs, iter->current_state,
                                     &next_limits, iter->bank,
                                     iter->subst, params, &moved_forward);
               if(moved_forward)
               {
                  assert(next_state != iter->current_state);
                  assert(next_state != DECOMPOSED_VAR);
                  prepare_backtrack(iter, lhs, rhs, next_state, iter->current_limits, subst_ptr);
                  iter->current_limits = next_limits;
                  iter->current_state = RIGID_PROCESSED_TAG; // first larger than INIT
                  // return the tasks
                  PQueueStoreP(iter->constraints, rhs);
                  PQueueStoreP(iter->constraints, lhs);
               }
               else if(GET_HEAD_ID(lhs) == GET_HEAD_ID(rhs))
               {
                  assert(lhs->arity == rhs->arity);
                  assert(TermIsPhonyApp(lhs) == TermIsPhonyApp(rhs));
                  schedule_args(iter, lhs->args+1, rhs->args+1, MAX(0, lhs->arity-1));
                  iter->current_state = RIGID_PROCESSED_TAG; // first larger than INIT
                  iter->current_limits = next_limits;
               }
               else
               {
                  res = backtrack_iter(iter);
               }
            }
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
            }
            else if (TermIsDBVar(rhs))
            {
               assert(!TermIsPhonyApp(lhs) && !TermIsDBVar(lhs));
               res = backtrack_iter(iter);
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
                  schedule_args(iter, lhs->args, rhs->args, lhs->arity);
               }
            }
            else
            {
               res = backtrack_iter(iter);
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
   bool res;
   if(iter->current_state == INIT_TAG)
   {
      assert(PQueueCardinality(iter->constraints) == 2);
      iter->current_state = RIGID_PROCESSED_TAG;
      res = true;
   }
   else
   {
      if(PStackEmpty(iter->backtrack_info) ||
         iter->unifiers_returned >= params->max_unifiers)
      {
         res = false;
      }
      else
      {
         assert(PStackGetSP(iter->backtrack_info) % BT_STEP_SIZE == 0);
         SubstBacktrackToPos(iter->subst, PStackPopInt(iter->backtrack_info));
         iter->current_limits = PStackPopInt(iter->backtrack_info);
         iter->current_state = PStackPopInt(iter->backtrack_info);
         PQueueFree(iter->constraints);
         iter->constraints = PStackPopP(iter->backtrack_info);
         res = true;
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
//   is destroyed and is no longer to be used.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool NextCSUElement(CSUIterator_p iter)
{
   bool res = backtrack_iter(iter);
   iter->steps = 0;
   if(res)
   {
      if(((problemType != PROBLEM_HO)||(params->unif_mode == SingleUnif))
         && iter->unifiers_returned == 0)
      {
         res = SubstMguComplete(PQueueGetLastP(iter->constraints),
                                PQueueGetLastP(iter->constraints),
                                iter->subst);
         // on the next call we destroy the iterator
         PStackReset(iter->backtrack_info);
         iter->unifiers_returned = 1;
      }
      else
      {
         res = forward_iter(iter);
         iter->unifiers_returned += res ? 1 : 0;
      }
   }
   if(!res)
   {
      SubstBacktrackToPos(iter->subst, iter->init_pos);
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
   assert(params);

   CSUIterator_p res = CSUIterAlloc();
   // fprintf(stderr, "begin:");
   // DBG_PRINT(stderr, "",  TermPrint(stderr, lhs, bank->sig, DEREF_NEVER), " <> ");
   // DBG_PRINT(stderr, "", TermPrint(stderr, rhs, bank->sig, DEREF_NEVER), ".\n");
   res->subst = subst;
   res->init_pos = PStackGetSP(subst);
   res->backtrack_info = PStackAlloc();
   res->constraints = PQueueAlloc();
   PQueueStoreP(res->constraints, rhs);
   PQueueStoreP(res->constraints, lhs);
   res->bank = bank;
   res->current_limits = 0;
   res->current_state = INIT_TAG;
   res->unifiers_returned = 0;
   res->steps = 0;

   // initialization of internal stufF
   res->tmp_rigid_diff = PStackAlloc();
   res->tmp_rigid_same = PStackAlloc();
   res->tmp_flex = PStackAlloc();
#ifndef NDEBUG
   res->orig_lhs = lhs;
   res->orig_rhs = rhs;
#endif
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

/*-----------------------------------------------------------------------
//
// Function: InitUnifLimits()
//
//   Store heuristic parameters locally and use them to pick up the
//   limits for unification.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void InitUnifLimits(HeuristicParms_p p)
{
   params = p;
}

/*-----------------------------------------------------------------------
//
// Function: CSUIterDestroy()
//
//  Destroys the iter and frees all the memory EXCEPT for the initial
//  substitution.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void CSUIterDestroy(CSUIterator_p iter)
{
   PStackPointer i = PStackGetSP(iter->backtrack_info);
   while(i>=BT_STEP_SIZE)
   {
      PQueueFree(PStackElementP(iter->backtrack_info, i-4));
      i-=4;
   }
   PStackFree(iter->backtrack_info);
   PQueueFree(iter->constraints);
   PStackFree(iter->tmp_rigid_diff);
   PStackFree(iter->tmp_rigid_same);
   PStackFree(iter->tmp_flex);
   SubstBacktrackToPos(iter->subst, iter->init_pos);
   SizeFree(iter, sizeof(CSUIterator_t));
}
