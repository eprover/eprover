/*-----------------------------------------------------------------------

File  : clb_objmaps.h

Author: Petar Vukmirovic

Contents

  Implements blocked clause elimination as described in 
  Blocked Clauses in First-Order Logic (https://doi.org/10.29007/c3wq).

Copyright 1998-2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> di  4 jan 2022 13:00:10 CET
-----------------------------------------------------------------------*/

#include "ccl_bce.h"
#include <clb_min_heap.h>

#define OCC_CNT(s) ((s) ? PStackGetSP(s) : 0)
#define IS_BLOCKED(s) ((s) && PStackGetSP(s) == 0)

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct 
{
   Clause_p orig_cl; // original clause that is potentially removed
   Clause_p parent; // renamed clause used for resulutons -- for efficiency
   Eqn_p lit;
   PStack_p candidates; // NB: Can be null;
   PStackPointer processed_cands;
} BCE_task;

typedef BCE_task* BCE_task_p;

typedef bool (*BlockednessChecker)(BCE_task_p, Clause_p, TB_p);

#define BCETaskFree(t) SizeFree((t), sizeof(BCE_task))

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
// Function: make_task()
// 
//   Makes the task for BCE.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

BCE_task_p make_task(Clause_p orig_cl, Clause_p cl, Eqn_p lit, PStack_p cands)
{
   BCE_task_p res = SizeMalloc(sizeof(BCE_task));
   res->orig_cl = orig_cl;
   res->parent = cl;
   res->lit = lit;
   res->candidates = cands;
   res->processed_cands = 0;
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: compare_taks()
// 
//   Function used to order tasks inside the task queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int compare_taks(IntOrP* ip_a, IntOrP* ip_b)
{
   BCE_task_p a = (BCE_task_p)ip_a;
   BCE_task_p b = (BCE_task_p)ip_b;
   // order by the number of remaining candidates
   return CMP((OCC_CNT(a->candidates)-a->processed_cands),
              (OCC_CNT(b->candidates)-b->processed_cands));
}

/*-----------------------------------------------------------------------
//
// Function: make_sym_map()
// 
//   Performs the elimination of blocked clauses by moving them
//   from passive to archive. Tracking a predicate symbol will be stopped
//   after it reaches max_occs occurrences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

IntMap_p make_sym_map(ClauseSet_p set, int occ_limit, bool* eq_found)
{
   IntMap_p res = IntMapAlloc();
   for(Clause_p cl = set->anchor->succ; cl!=set->anchor; cl = cl->succ)
   {
      for(Eqn_p lit=cl->literals; lit; lit = lit->next)
      {
         if(!EqnIsEquLit(lit))
         {
            FunCode fc = lit->lterm->f_code * (EqnIsPositive(lit) ? 1 : -1);
            PStack_p* fc_cls = (PStack_p*)IntMapGetRef(res, fc);
            if(!IS_BLOCKED(*fc_cls))
            {
               PStack_p other_fc_cls = (PStack_p)IntMapGetVal(res, -fc);
               if( occ_limit > 0 && // limiting enabled
                  ((OCC_CNT(*fc_cls) + OCC_CNT(other_fc_cls)) >= occ_limit))
               {
                  // removing all elements -- essentially blocking clauses
                  if(*fc_cls)
                  {
                     PStackReset(*fc_cls);
                  }
                  else
                  {
                     *fc_cls = PStackAlloc();
                  }

                  PStack_p* other_fc_ref = (PStack_p*)IntMapGetRef(res, -fc);
                  if(*other_fc_ref)
                  {
                     PStackReset(*other_fc_ref);
                  }
                  else
                  {
                     *other_fc_ref = PStackAlloc();
                  }
               }
               else
               {
                  if(!*fc_cls)
                  {
                     *fc_cls = PStackAlloc();
                     PStackPushP(*fc_cls, cl);
                  }
                  else if(PStackTopP(*fc_cls) != cl)
                  {
                     // putting only one copy of a clause
                     PStackPushP(*fc_cls, cl);
                  }
               }
            }
         }
         else
         {
            *eq_found = true;
         }
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: make_bce_queue()
// 
//   For each literal in a clause build an object encapsulating
//   all the candidates (and how far we are in checking them), then
//   store it in a queue ordered by number of candidates to check.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

MinHeap_p make_bce_queue(ClauseSet_p set, IntMap_p sym_map, PStack_p fresh_clauses)
{
   MinHeap_p res = MinHeapAlloc(compare_taks);
   for(Clause_p cl = set->anchor->succ; cl!=set->anchor; cl = cl->succ)
   {
      assert(cl->set);
      Clause_p f_cl = ClauseCopyDisjoint(cl);
      for(Eqn_p lit=f_cl->literals; lit; lit = lit->next)
      {
         if(!EqnIsEquLit(lit))
         {
            FunCode fc = lit->lterm->f_code * (EqnIsPositive(lit) ? 1 : -1);
            PStack_p cands = IntMapGetVal(sym_map, -fc);
            if(!IS_BLOCKED(cands))
            {
               BCE_task_p t = make_task(cl, f_cl, lit, cands);
               MinHeapAddP(res, t);
            }
         }
      }  
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: split_partner_literals()
// 
//   Splits the literals in partner clause into those that unify and
//   those that do not unify with "lit".
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void split_partner_literals(Eqn_p lit, Clause_p partner,
                            PStack_p unif, PStack_p non_unif)
{
   Subst_p subst = SubstAlloc();
   for(Eqn_p part_lit = partner->literals; part_lit; part_lit = part_lit->next)
   {
      if(EqnIsPositive(lit) != EqnIsPositive(part_lit) &&
         !EqnIsEquLit(part_lit) && 
         SubstMguComplete(lit->lterm, part_lit->lterm, subst))
      {
         PStackPushP(unif, part_lit);
         SubstBacktrack(subst);
      }
      else
      {
         PStackPushP(non_unif, part_lit);
      }

   }
   SubstDelete(subst);
}

/*-----------------------------------------------------------------------
//
// Function: check_blockedness_eq()
// 
//   Check if clause all equational L-resolvents between literal
//   described by task and b are tautologies.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool check_l_resolvents_neq(Clause_p cl, Eqn_p lit, PStack_p unif,
                            PStack_p non_unif, PStack_p processed)
{
   assert(!PStackEmpty(processed));
   Subst_p subst = SubstAlloc();
   Eqn_p part_lit = PStackElementP(unif, PStackTopInt(processed));
#ifndef NDEBUG
   bool unif_res = 
#endif
      SubstMguComplete(lit->lterm, part_lit->lterm, subst)
   ;
   assert(unif_res);
   bool res = false;

   while(true)
   {
      bool comp_found = false;
      for(PStackPointer i=0; !comp_found && i<PStackGetSP(non_unif); i++)
      {
         comp_found = EqnListFindCompLitExcept(cl->literals, lit, 
                                               PStackElementP(non_unif, i),
                                               DEREF_ALWAYS, DEREF_ALWAYS);
      }
      
      if(comp_found)
      {
         // found complementing literals outside L-group of literals,
         // no need for extending the processed group
         res = true;
         break;
      }

      if(PStackGetSP(processed) == PStackGetSP(unif))
      {
         // went through all the unifiable literals, cannot be extended anymore
         break;
      }

      PStackPointer prev_try = PStackGetSP(processed);
      for(PStackPointer i=0; i<PStackGetSP(unif); i++)
      {
         if(!PStackFindInt(processed, i))
         {
            if(EqnListFindCompLitExcept(cl->literals, lit, 
                                        PStackElementP(unif, i),
                                        DEREF_ALWAYS, DEREF_ALWAYS))
            {
               PStackPushInt(processed, i);
            }
         }
      }

      if(prev_try == PStackGetSP(processed))
      {
         // not extended, did not find complementary literals
         break;
      }
      
      bool unifiable = true;
      for(; unifiable && prev_try < PStackGetSP(processed); prev_try++)
      {
         Eqn_p o_lit = PStackElementP(unif, PStackElementInt(processed, prev_try));
         unifiable = SubstMguComplete(lit->lterm, o_lit->lterm, subst);
      }

      if(!unifiable)
      {
         res = true;
         break;
      }
   }

   SubstDelete(subst);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: check_blockedness_neq()
// 
//   Check if clause all L-resolvents between literal described by task
//   and b are tautologies.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool check_blockedness_neq(BCE_task_p task, Clause_p partner, TB_p _)
{
   assert(!EqnIsEquLit(task->lit));
   Eqn_p lit = task->lit;
   bool res;
   
   PStack_p unifiable = PStackAlloc();
   PStack_p nonunifiable = PStackAlloc();
   split_partner_literals(lit, partner, unifiable, nonunifiable);

   PStack_p processed = PStackAlloc();
   res = true;
   for(PStackPointer i=0; res && i<PStackGetSP(unifiable); i++)
   {
      PStackReset(processed);
      PStackPushInt(processed, i);
      res = check_l_resolvents_neq(task->parent, lit, unifiable,
                                    nonunifiable, processed);
   }
   
   PStackFree(processed);
   PStackFree(unifiable);
   PStackFree(nonunifiable);
   
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: check_blockedness_eq()
// 
//   Check if clause all equational L-resolvents between literal
//   described by task and b are tautologies.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool check_blockedness_eq(BCE_task_p task, Clause_p partner, TB_p tmp_bank)
{
   assert(!EqnIsEquLit(task->lit));
   Eqn_p lit = task->lit;
   bool res;
   
   PStack_p same_head = PStackAlloc();
   Eqn_p others = NULL;

   for(Eqn_p p_lit = partner->literals; p_lit; p_lit = p_lit->next)
   {
      if(EqnIsPositive(lit) != EqnIsPositive(p_lit) &&
         !EqnIsEquLit(p_lit) &&
         lit->lterm->f_code == p_lit->lterm->f_code)
      {
         PStackPushP(same_head, p_lit);
      }
      else
      {
         Eqn_p eq_copy = EqnCopy(p_lit, p_lit->bank);
         EqnListInsertFirst(&others, eq_copy);
      }
   }

   res = true;
   while(res && !PStackEmpty(same_head))
   {
      Eqn_p p_lit = PStackPopP(same_head);
      Eqn_p cond = NULL;
      for(int i=0; i<p_lit->lterm->arity; i++)
      {
         Eqn_p neq = EqnAlloc(p_lit->lterm->args[i], lit->lterm->args[i],
                              p_lit->bank, false);
         EqnListInsertFirst(&cond, neq);
      }
      Eqn_p orig_cl = EqnListCopyExcept(task->parent->literals, 
                                        task->lit, task->lit->bank);
      EqnListAppend(&cond, orig_cl);
      EqnListAppend(&cond, EqnListCopy(others, others->bank));
      Clause_p tmp_cl = ClauseAlloc(cond);
      res = ClauseIsTautologyReal(tmp_bank, tmp_cl, false);
      ClauseFree(tmp_cl);
   }

   PStackFree(same_head);
   EqnListFree(others);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: check_candidates()
// 
//   Forwards the task either to the first clause that makes it non-blocked.
//   Otherwise, forwards it to the end of the candidates list.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void check_candidates(BCE_task_p t, ClauseSet_p archive, 
                      BlockednessChecker f, TB_p tmp_bank)
{
   assert(!IS_BLOCKED(t->candidates));

   for(;t->processed_cands < OCC_CNT(t->candidates); t->processed_cands++)
   {
      Clause_p cand = PStackElementP(t->candidates, t->processed_cands);
      if(cand != t->orig_cl && cand->set != archive && !f(t, cand, tmp_bank))
      {
         break;
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: resume_task()
// 
//   Forwards to the next candidate and reinserts the task into the queue.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void resume_task(MinHeap_p task_queue, BCE_task_p task)
{
   task->processed_cands++;
   MinHeapAddP(task_queue, task);
}

/*-----------------------------------------------------------------------
//
// Function: do_eliminate_clauses()
// 
//   Performs actual clause elimination
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void free_blocker(void *key, void* val)
{
   PStack_p blocked_tasks = (PStack_p)val;
   while(!PStackEmpty(blocked_tasks))
   {
      BCE_task_p t = PStackPopP(blocked_tasks);
      SizeFree(t, sizeof(BCE_task));
   }
   PStackFree(blocked_tasks);
}

long do_eliminate_clauses(MinHeap_p task_queue, ClauseSet_p archive, 
                          bool has_eq, TB_p tmp_bank)
{
   PObjMap_p blocker_map = NULL;
   BlockednessChecker checker = 
      has_eq ? check_blockedness_eq : check_blockedness_neq;
   long eliminated = 0;
   while(MinHeapSize(task_queue))
   {
      BCE_task_p min_task = MinHeapPopMaxP(task_queue);
      if(min_task->orig_cl->set != archive)
      {
         // clause is not archived, we can go on
         check_candidates(min_task, archive, checker, tmp_bank);
         if(min_task->processed_cands == OCC_CNT(min_task->candidates))
         {
            // all candidates are processed, clause is blocked
            ClauseSetMoveClause(archive, min_task->orig_cl);
            eliminated++;
            DBG_PRINT(stderr, "eliminated ", ClausePrint(stderr, min_task->orig_cl, true), ".\n");

            PStack_p blocked = PObjMapExtract(&blocker_map, min_task->parent, PCmpFun);
            if(blocked)
            {
               while(!PStackEmpty(blocked))
               {
                  resume_task(task_queue, PStackPopP(blocked));
               }
               PStackFree(blocked);
            }
            BCETaskFree(min_task);
         }
         else
         {
            // because of the last checked candidate, clause is not blocked.
            // remember that checking of candidates needs to be continued
            // once the clause which prevented blocking is removed
            PStack_p* blocked = (PStack_p*)PObjMapGetRef(&blocker_map, min_task->parent, PCmpFun, NULL);
            if(!*blocked)
            {
               *blocked = PStackAlloc();
            }
            PStackPushP(*blocked, min_task);
         }
      }
   }
   PObjMapFreeWDeleter(blocker_map, free_blocker);
   return eliminated;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: EliminateBlockedClauses()
// 
//   Performs the elimination of blocked clauses by moving them
//   from passive to archive. Tracking a predicate symbol will be stopped
//   after it reaches max_occs occurrences.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void EliminateBlockedClauses(ClauseSet_p passive, ClauseSet_p archive,
                             int max_occs, TB_p tmp_bank)
{
   bool eq_found = false;
   IntMap_p sym_occs = make_sym_map(passive, max_occs, &eq_found);
   IntMapIter_p iter;
   long key;
   PStack_p cls = NULL;

   PStack_p fresh_cls = PStackAlloc();
   MinHeap_p task_queue = make_bce_queue(passive, sym_occs, fresh_cls);
   long num_eliminated = 
      do_eliminate_clauses(task_queue, archive, eq_found, tmp_bank);
   
   iter = IntMapIterAlloc(sym_occs, LONG_MIN, LONG_MAX);
   cls = NULL;
   while( (cls = IntMapIterNext(iter, &key)) )
   {
      PStackFree(cls);
   }
   IntMapIterFree(iter);

   while(!PStackEmpty(fresh_cls))
   {
      ClauseFree(PStackPopP(fresh_cls));
   }

   PStackFree(fresh_cls);

}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/



