/*-----------------------------------------------------------------------

  File  : ccl_subsumption.c

  Author: Stephan Schulz

  Contents

  Functions for subsumption of clauses.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sun Jun  7 15:12:29 MET DST 1998

  -----------------------------------------------------------------------*/

#include "ccl_subsumption.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(SetSubsumeTimer);
PERF_CTR_DEFINE(SubsumeTimer);

bool StrongUnitForwardSubsumption     = false;
long ClauseClauseSubsumptionCalls     = 0;
long ClauseClauseSubsumptionCallsRec  = 0;
long ClauseClauseSubsumptionSuccesses = 0;
long UnitClauseClauseSubsumptionCalls = 0;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: unit_clause_set_strongsubsumes_termpair()
//
//   Return a unit clause with sign positive from set if there is a
//   subset with sign positive that shows t1=t2 in one step. Return
//   NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
ClausePos_p unit_clause_set_strongsubsumes_termpair(ClauseSet_p set,
                      Term_p t1, Term_p t2,
                      bool positive)
{
   PStack_p stack = PStackAlloc();
   int      i;
   ClausePos_p res = NULL;

   PStackPushP(stack, t1);
   PStackPushP(stack, t2);

   while(!PStackEmpty(stack))
   {
      t2 = PStackPopP(stack);
      t1 = PStackPopP(stack);
      res = FindSignedTopSimplifyingUnit(set, t1, t2, positive);

      if(SimplifyFailed(res))
      {
         if(TermIsAppliedFreeVar(t1) || TermIsAppliedFreeVar(t2) ||
            TermIsLambda(t1) || TermIsLambda(t2) ||
            t1->f_code != t2->f_code || !t1->arity)
         {
            break;
         }

         assert(t1->type == t2->type);
         for(i=0; i<t1->arity; i++)
         {
            if(t1->args[i] != t2->args[i])
            {
               PStackPushP(stack, t1->args[i]);
               PStackPushP(stack, t2->args[i]);
            }
         }
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: unit_clause_set_subsumes_clause()
//
//   Return a clause from set that subsumes clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p unit_clause_set_subsumes_clause(ClauseSet_p set,
                Clause_p clause)
{
   Eqn_p    handle = clause->literals;
   ClausePos_p res = NULL;

   while(handle)
   {
      if(EqnIsPositive(handle))
      {
         res = StrongUnitForwardSubsumption?
                  unit_clause_set_strongsubsumes_termpair(set, handle->lterm,
                                                          handle->rterm,
                                                          true):
                  FindSimplifyingUnit(set, handle->lterm,
                                      handle->rterm,
                                      true);
      }
      else
      {
         res = FindSignedTopSimplifyingUnit(set,
                                            handle->lterm,
                                            handle->rterm,
                                            false);
      }

      if(!SimplifyFailed(res))
      {
         break;
      }
      handle = handle->next;
   }
   return res ? res->clause : NULL;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_topsubsumes_termpair()
//
//   Return true if eqn subsumes t1=t2 at top level.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool eqn_topsubsumes_termpair(Eqn_p eqn, Term_p t1, Term_p t2)
{
   Subst_p subst = SubstAlloc();
   bool    res = false;

   assert(eqn);
   assert(t1);
   assert(t2);

   if(SubstMatchComplete(eqn->lterm, t1, subst))
   {
      if(SubstMatchComplete(eqn->rterm, t2, subst))
      {
         res = true;
      }
   }
   else if(SubstMatchComplete(eqn->lterm, t2, subst))
   {
      if(SubstMatchComplete(eqn->rterm, t1, subst))
      {
         res = true;
      }
   }
   SubstDelete(subst);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_subsumes_termpair()
//
//   Return true if the equation subsumes t1=t2.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool eqn_subsumes_termpair(Eqn_p eqn, Term_p t1, Term_p t2)
{
   Term_p   tmp1, tmp2 = NULL;
   int      i;
   bool     res = false;

   assert(t1);
   assert(t2);
   assert(eqn);

   while(!(res = eqn_topsubsumes_termpair(eqn, t1, t2)))
   {
      if(TermIsPhonyApp(t1) || TermIsPhonyApp(t2) ||
         t1->f_code != t2->f_code || !t1->arity)
      {
         break;
      }
#ifdef ENABLE_LFHO
      if(t1->arity != t2->arity)
      {
        break; // do not try arguments if the number does not match
      }
#else
      assert(t1->arity == t2->arity);
#endif

      tmp1 = NULL;
      tmp2 = NULL;

      for(i=0; i<t1->arity; i++)
      {
         if(t1->args[i] != t2->args[i])
         {
            if(tmp1)
            {
               return false;
            }
            tmp1 = t1->args[i];
            tmp2 = t2->args[i];
         }
      }
      if(!tmp1)
      {
         return true;
      }
      t1 = tmp1;
      t2 = tmp2;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: find_spec_literal()
//
//   Find a literal in list that is more special than lit. Return it
//   or NULL if none exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#ifndef NDEBUG
/* Old version used for comparison only */
Eqn_p find_spec_literal_old(Eqn_p lit, Eqn_p list)
{
   Subst_p subst = SubstAlloc();

   for(;list;list = list->next)
   {
      if(!PropsAreEquiv(lit, list, EPIsPositive|EPIsEquLiteral))
      {
         continue;
      }
      if(EqnIsOriented(lit) && !EqnIsOriented(list))
      {
         continue;
      }
      if(SubstMatchComplete(lit->lterm, list->lterm, subst)&&
         SubstMatchComplete(lit->rterm, list->rterm, subst))
      {
         break;
      }
      SubstBacktrack(subst);
      if(EqnIsOriented(lit))
      {
         continue;
      }
      if(SubstMatchComplete(lit->lterm, list->rterm, subst)&&
         SubstMatchComplete(lit->rterm, list->lterm, subst))
      {
         break;
      }
      SubstBacktrack(subst);
   }
   SubstDelete(subst);
   return list;
}
#endif

/* New version using ordering */
static Eqn_p find_spec_literal(Eqn_p lit, Eqn_p list)
{
   Subst_p subst = SubstAlloc();
   int cmpres;

   for(;list;list = list->next)
   {
      cmpres = EqnSubsumeQOrderCompare(lit, list);
      if(cmpres > 0)
      {
         list = NULL;
         break;
      }
      if(cmpres <  0)
      {
         continue;
      }
      if(EqnStandardWeight(lit) > EqnStandardWeight(list))
      {
         list = NULL;
         break;
      }
      assert(PropsAreEquiv(lit, list, EPIsPositive|EPIsEquLiteral));
      if(EqnIsOriented(lit) && !EqnIsOriented(list))
      {
         continue;
      }
      if(SubstMatchComplete(lit->lterm, list->lterm, subst)&&
         SubstMatchComplete(lit->rterm, list->rterm, subst))
      {
         break;
      }
      SubstBacktrack(subst);
      if(EqnIsOriented(lit))
      {
         continue;
      }
      if(SubstMatchComplete(lit->lterm, list->rterm, subst)&&
         SubstMatchComplete(lit->rterm, list->lterm, subst))
      {
         break;
      }
      SubstBacktrack(subst);
   }
   SubstDelete(subst);
   return list;
}


/*-----------------------------------------------------------------------
//
// Function: check_subsumption_possibility()
//
//   Return true if each literal in subsumer is more general than a
//   literal in sub_candidate.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool check_subsumption_possibility(Clause_p subsumer, Clause_p
                                          sub_candidate)
{
   bool    res = true;
   Eqn_p   sub_eqn;

   for(sub_eqn = subsumer->literals; sub_eqn; sub_eqn = sub_eqn->next)
   {
      if(!find_spec_literal(sub_eqn, sub_candidate->literals))
      {
         res = false;
         break;
      }
   }
   return res;
}
#ifdef NEVER_DEFINED
{
   bool    res = true;
   Eqn_p   sub_eqn;
   PStack_p lit_stack = ClauseToStack(subsumer);

   while(!PStackEmpty(lit_stack))
   {
      sub_eqn = PStackPopP(lit_stack);
      if(!find_spec_literal(sub_eqn, sub_candidate->literals))
      {
         res = false;
         break;
      }
   }
   PStackFree(lit_stack);
   return res;
}
#endif


/*-----------------------------------------------------------------------
//
// Function: eqn_list_rec_subsume()
//
//   Try to find a subset of sub_cand_list such that
//   subst(subsum_list) = subset. Return true if this is possible,
//   false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

/* Old version !*/
#ifndef NDEBUG
bool eqn_list_rec_subsume_old(Eqn_p subsum_list, Eqn_p sub_cand_list,
                              Subst_p subst, long* pick_list)
{
   Eqn_p         eqn;
   PStackPointer state;
   int lcount;

   if(!subsum_list)
   {
      /* return check_subsumption_condition(sub_cand_list, subst,
       * pick_list);*/
      return true;
   }

   for(eqn = sub_cand_list, lcount=0; eqn; eqn = eqn->next, lcount++)
   {
      /* We now use strict multiset-subsumption. I should probably
         rewrite this code to be more efficient for that case...*/

      if(pick_list[lcount])
      {
         continue;
      }

      /* Some optimizations: Of course both equation need to have the
         same sign. If the potentially more general equation
         is oriented, then the potentially more specialized has to be
         oriented as well. Also, if the potentially more specialized
         equation is maximal, so has to be the more general one. */
      if(!PropsAreEquiv(eqn, subsum_list, EPIsPositive|EPIsEquLiteral))
      {
         continue;
      }
      if(EqnIsOriented(subsum_list) && !EqnIsOriented(eqn))
      {
         continue;
      }
      /* This assumption is no longer valid with selection (selection
         works by making arbitrary negative literals
         maximal). Moreover, for some strange reason it also slowed
         down the ordinary case */
      /* if(EqnIsMaximal(eqn) && !EqnIsMaximal(subsum_list))
         {
            continue;
         }  */
      pick_list[lcount]++;
      state = PStackGetSP(subst);

      if(SubstMatchComplete(subsum_list->lterm, eqn->lterm, subst)&&
         SubstMatchComplete(subsum_list->rterm, eqn->rterm, subst))
      {
         if(eqn_list_rec_subsume_old(subsum_list->next, sub_cand_list,
                     subst, pick_list))
         {
            return true;
         }
      }
      SubstBacktrackToPos(subst, state);
      if(EqnIsOriented(subsum_list))
      {
         state = PStackGetSP(subst);
         pick_list[lcount]--;
         continue;
      }
      if(SubstMatchComplete(subsum_list->lterm, eqn->rterm, subst)&&
         SubstMatchComplete(subsum_list->rterm, eqn->lterm, subst))
      {
         if(eqn_list_rec_subsume_old(subsum_list->next, sub_cand_list,
                                     subst, pick_list))
         {
            return true;
         }
      }
      SubstBacktrackToPos(subst, state);
      pick_list[lcount]--;
   }
   return false;
}
#endif

static
bool eqn_list_rec_subsume(Eqn_p subsum_list, Eqn_p sub_cand_list,
                          Subst_p subst, long* pick_list)
{
   Eqn_p         eqn;
   PStackPointer state;
   int lcount, cmpres;

   if(!subsum_list)
   {
      return true;
   }

   for(eqn = sub_cand_list, lcount=0; eqn; eqn = eqn->next, lcount++)
   {
      /* We now use strict multiset-subsumption. I should probably
         rewrite this code to be more efficient for that case...*/
      if(pick_list[lcount])
      {
         continue;
      }

      cmpres = EqnSubsumeQOrderCompare(eqn,subsum_list);
      if(cmpres < 0)
      {
         return false;
      }
      if(cmpres >  0)
      {
         continue;
      }

      if(EqnStandardWeight(eqn) < EqnStandardWeight(subsum_list))
      {
         return false;
      }
      assert(PropsAreEquiv(subsum_list, eqn, EPIsPositive|EPIsEquLiteral));
      /* Some optimizations:If the potentially more general equation
         is oriented, then the potentially more specialized has to be
         oriented as well. */
      if(EqnIsOriented(subsum_list) && !EqnIsOriented(eqn))
      {
         continue;
      }

      pick_list[lcount]++;
      state = PStackGetSP(subst);

      if(SubstMatchComplete(subsum_list->lterm, eqn->lterm, subst)&&
         SubstMatchComplete(subsum_list->rterm, eqn->rterm, subst))
      {
         if(eqn_list_rec_subsume(subsum_list->next, sub_cand_list,
                                 subst, pick_list))
         {
            return true;
         }
      }
      SubstBacktrackToPos(subst, state);
      if(EqnIsOriented(subsum_list))
      {
         state = PStackGetSP(subst);
         pick_list[lcount]--;
         continue;
      }
      if(SubstMatchComplete(subsum_list->lterm, eqn->rterm, subst)&&
         SubstMatchComplete(subsum_list->rterm, eqn->lterm, subst))
      {
         if(eqn_list_rec_subsume(subsum_list->next, sub_cand_list,
                                 subst, pick_list))
         {
            return true;
         }
      }
      SubstBacktrackToPos(subst, state);
      pick_list[lcount]--;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: clause_subsumes_clause()
//
//   Return true if subsumer subsumes sub_candidate. Assumes that
//   weights are precomputed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool clause_subsumes_clause(Clause_p subsumer, Clause_p
                                   sub_candidate)
{
   Subst_p subst;
   bool    res;
   long* pick_list;

   PERF_CTR_ENTRY(SubsumeTimer);

   /* if(!ClauseIsSubsumeOrdered(sub_candidate)) */
   /* { */
   /*    printf(COMCHAR" sub_candidate %p: ", sub_candidate->set);ClausePrint(stdout, sub_candidate, true); */
   /*    printf("\n"COMCHAR" subsumer     %p: ", subsumer->set);ClausePrint(stdout, subsumer, true); */
   /*    printf("\n"); */

   /*    printf(COMCHAR" %ld, %ld\n", EqnStandardWeight(sub_candidate->literals),EqnStandardWeight(sub_candidate->literals->next)); */
   /*    printf(COMCHAR" EqnSubsumeInverseCompareRef()=%d\n", EqnSubsumeInverseCompareRef(&(sub_candidate->literals), &(sub_candidate->literals->next))); */
   /*    printf(COMCHAR" EqnSubsumeQOrderCompare()=%d\n", EqnSubsumeQOrderCompare(sub_candidate->literals, sub_candidate->literals->next)); */
   /*    ClauseSubsumeOrderSortLits(sub_candidate); */
   /*    printf(COMCHAR" sub_candidate %p: ", sub_candidate->set);ClausePrint(stdout, sub_candidate, true); */
   /*    printf("\n"); */
   /* } */
   assert(ClauseIsSubsumeOrdered(subsumer));
   assert(ClauseIsSubsumeOrdered(sub_candidate));

   if(ClauseLiteralNumber(subsumer)==0)
   {
      PERF_CTR_EXIT(SubsumeTimer);
      return true;
   }
   if(ClauseLiteralNumber(subsumer)==1)
   {
      PERF_CTR_EXIT(SubsumeTimer);
      return UnitClauseSubsumesClause(subsumer, sub_candidate);
   }
   /*fprintf(stderr, COMCHAR" sub_candidate:");ClausePrint(stderr, sub_candidate, true);
   fprintf(stderr, "\n"COMCHAR" subsumer:");ClausePrint(stderr, subsumer, true);
   fprintf(stderr, "\n");*/

   assert(sub_candidate->weight == ClauseStandardWeight(sub_candidate));
   assert(subsumer->weight == ClauseStandardWeight(subsumer));

   ClauseClauseSubsumptionCalls++;

   if((subsumer->pos_lit_no > sub_candidate->pos_lit_no) ||
      (subsumer->neg_lit_no > sub_candidate->neg_lit_no))
   {
      PERF_CTR_EXIT(SubsumeTimer);
      return false;
   }
   if(subsumer->weight > sub_candidate->weight)
   {
      PERF_CTR_EXIT(SubsumeTimer);
      return false;
   }
   if(((sub_candidate->pos_lit_no >=3) ||
       (sub_candidate->neg_lit_no >=3))&&
      !check_subsumption_possibility(subsumer, sub_candidate))
   {
      PERF_CTR_EXIT(SubsumeTimer);
      return false;
   }
   subst = SubstAlloc();
   ClauseClauseSubsumptionCallsRec++;

   pick_list = IntArrayAlloc(ClauseLiteralNumber(sub_candidate));

   res = eqn_list_rec_subsume(subsumer->literals,
                              sub_candidate->literals, subst,
                              pick_list);
   IntArrayFree(pick_list, ClauseLiteralNumber(sub_candidate));

   SubstDelete(subst);

   PERF_CTR_EXIT(SubsumeTimer);

   if(res)
   {
      ClauseClauseSubsumptionSuccesses++;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: plain_lit_matches()
//
//   Try to extend subst to a match from pattern onto target (with
//   the sides of target swapped if swap is true). Return true on
//   success. On failure, subst is unchanged.
//
// Global Variables: -
//
// Side Effects    : Extends subst
//
/----------------------------------------------------------------------*/

static bool plain_lit_matches(Eqn_p pattern, Eqn_p target, bool swap,
                              Subst_p subst)
{
   PStackPointer state = PStackGetSP(subst);
   Term_p lterm = swap? target->rterm : target->lterm;
   Term_p rterm = swap? target->lterm : target->rterm;

   if(SubstMatchComplete(pattern->lterm, lterm, subst) &&
      SubstMatchComplete(pattern->rterm, rterm, subst))
   {
      return true;
   }
   SubstBacktrackToPos(subst, state);
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_list_rec_plain_subsume()
//
//   Return true if subst can be extended to a substitution that maps
//   every literal of patterns onto some literal of targets other than
//   except (which may be NULL). Several patterns may be mapped onto
//   the same target (plain (set) subsumption, not multiset
//   subsumption). Equations are matched in both orientations. On
//   failure, subst is unchanged. On success, the caller has to
//   backtrack subst.
//
// Global Variables: -
//
// Side Effects    : Extends subst
//
/----------------------------------------------------------------------*/

static bool eqn_list_rec_plain_subsume(Eqn_p patterns, Eqn_p targets,
                                       Eqn_p except, Subst_p subst)
{
   Eqn_p         target;
   PStackPointer state = PStackGetSP(subst);

   if(!patterns)
   {
      return true;
   }
   for(target = targets; target; target = target->next)
   {
      if((target == except) ||
         !PropsAreEquiv(patterns, target, EPIsPositive|EPIsEquLiteral))
      {
         continue;
      }
      if(plain_lit_matches(patterns, target, false, subst))
      {
         if(eqn_list_rec_plain_subsume(patterns->next, targets, except,
                                       subst))
         {
            return true;
         }
         SubstBacktrackToPos(subst, state);
      }
      if(EqnIsEquLit(patterns) &&
         plain_lit_matches(patterns, target, true, subst))
      {
         if(eqn_list_rec_plain_subsume(patterns->next, targets, except,
                                       subst))
         {
            return true;
         }
         SubstBacktrackToPos(subst, state);
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// The following functions search clause sets and FV indices for
// subsuming or subsumed clauses. The subsumption test is passed in
// as subsumes (multiset or plain subsumption). Note that the FV
// index filters candidates under the assumption of multiset
// subsumption, so with plain subsumption it may miss some.
//
/----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: clause_set_subsumes_clause()
//
//   Return subsuming clause if the set subsumes sub_candidate, NULL
//   otherwise. sub_candidate may be a unit clause. All clauses need
//   correct weights!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p clause_set_subsumes_clause(ClauseSet_p set, Clause_p sub_candidate,
                                    ClauseSubsumesFun subsumes)
{
   Clause_p handle;

   assert(sub_candidate->weight ==
          ClauseStandardWeight(sub_candidate));

   for(handle = set->anchor->succ; handle != set->anchor;
       handle = handle->succ)
   {
      if(subsumes(handle, sub_candidate))
      {
         // ClauseSetProp(handle, ClauseQueryProp(sub_candidate,CPIsSOS));
         // Not a SOS clause!
         return handle;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: clause_tree_find_subsuming_clause()
//
//   Given a set of clauses and a clause, return the first subsuming
//   clause (in the order of the set) or NULL
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static
Clause_p clause_tree_find_subsuming_clause(OrdSet_p tree, Clause_p sub_candidate,
                                           ClauseSubsumesFun subsumes)
{
   OrdSet_p handle;
   Clause_p clause;

   assert(sub_candidate->weight == ClauseStandardWeight(sub_candidate));

   for(handle = OrdSetFirst(tree); handle; handle = OrdSetNext(tree, handle))
   {
      clause = handle->key;
      if(subsumes(clause,sub_candidate))
      {
         /* DocClauseQuote(GlobalOut, OutputLevel, 6, sub_candidate,
            "subsumed", clause); */
         //ClauseSetProp(clause, ClauseQueryProp(sub_candidate,CPIsSOS));
         // Not a SOS clause!
         return clause;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: clause_set_subsumes_clause_indexed()
//
//   Return a clause from the index that subsumes sub_candidate (with
//   feature vector vec), NULL if there is none. All clauses need
//   correct weights!
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static
Clause_p clause_set_subsumes_clause_indexed(FVIndex_p index,
                                            Clause_p sub_candidate,
                                            FreqVector_p vec,
                                            long feature,
                                            ClauseSubsumesFun subsumes)
{

   assert(sub_candidate->weight == ClauseStandardWeight(sub_candidate));

   if(feature == vec->size)
   {
      return clause_tree_find_subsuming_clause(index->u1.clauses,
                                               sub_candidate, subsumes);
   }
   else if(index->u1.successors)
   {
      long i;
      FVIndex_p next;
      IntMapIter_p iter;
      Clause_p res = NULL;

      iter = IntMapIterAlloc(index->u1.successors, 0, vec->array[feature]);
      while((next = IntMapIterNext(iter, &i)))
      {
         if(next->clause_count &&
            (res = clause_set_subsumes_clause_indexed(next, sub_candidate,
                                                      vec, feature+1, subsumes)))
         {
            break;
         }
      }
      IntMapIterFree(iter);
      return res;
   }
   return NULL;
}

/*-----------------------------------------------------------------------
//
// Function: clause_tree_find_subsumed_clauses()
//
//   Given a set of clauses and a clause, push all subsumed clauses
//   onto res (in the order of the set).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static
void clause_tree_find_subsumed_clauses(OrdSet_p tree, Clause_p subsumer,
                                       PStack_p res,
                                       ClauseSubsumesFun subsumes)
{
   OrdSet_p handle;
   Clause_p clause;

   assert(subsumer->weight == ClauseStandardWeight(subsumer));

   for(handle = OrdSetFirst(tree); handle; handle = OrdSetNext(tree, handle))
   {
      clause = handle->key;
      if(subsumes(subsumer, clause))
      {
         /* DocClauseQuote(GlobalOut, OutputLevel, 6, clause,
            "subsumed", subsumer);*/
         //ClauseSetProp(subsumer, ClauseQueryProp(clause,CPIsSOS));
         // Not a SOS clause!
         PStackPushP(res, clause);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: clause_tree_find_first_subsumed_clause()
//
//   Given a set of clauses and a clause, return the first clause in
//   the set (in its order) subsumed by the clause, or NULL.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static
Clause_p clause_tree_find_first_subsumed_clause(OrdSet_p tree,
                                            Clause_p subsumer,
                                                ClauseSubsumesFun subsumes)
{
   OrdSet_p handle;
   Clause_p clause;

   assert(subsumer->weight == ClauseStandardWeight(subsumer));

   for(handle = OrdSetFirst(tree); handle; handle = OrdSetNext(tree, handle))
   {
      clause = handle->key;
      if(subsumes(subsumer, clause))
      {
         /* DocClauseQuote(GlobalOut, OutputLevel, 6, clause,
            "subsumed", subsumer);*/
         //ClauseSetProp(subsumer, ClauseQueryProp(clause,CPIsSOS));
         // Not a SOS clause!
         return clause;
      }
   }
   return NULL;
}



/*-----------------------------------------------------------------------
//
// Function: clauseset_find_subsumed_clauses()
//
//   Find all clauses subsumed by subsumer and push them onto
//   stack. Also write PCL statements to that effect (if required by
//   output level).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static
void clauseset_find_subsumed_clauses(ClauseSet_p set,
                                     Clause_p subsumer,
                                     PStack_p res,
                                     ClauseSubsumesFun subsumes)
{
   Clause_p handle;

   for(handle = set->anchor->succ;
       handle!= set->anchor;
       handle = handle->succ)
   {
      if(subsumes(subsumer, handle))
      {
         /* DocClauseQuote(GlobalOut, OutputLevel, 6, handle,
            "subsumed", subsumer); */
         PStackPushP(res, handle);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: clauseset_find_first_subsumed_clause()
//
//   Find first subsumed clause in set and return it (or NULL, if no
//   such clause exists).
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static
Clause_p clauseset_find_first_subsumed_clause(ClauseSet_p set,
                                              Clause_p subsumer,
                                              ClauseSubsumesFun subsumes)
{
   Clause_p handle;

   for(handle = set->anchor->succ;
       handle!= set->anchor;
       handle = handle->succ)
   {
      if(subsumes(subsumer, handle))
      {
         return handle;
      }
   }
   return NULL;
}



/*-----------------------------------------------------------------------
//
// Function: clauseset_find_subsumed_clauses_indexed()
//
//   Find all clauses in index subsumed by subsumer (with feature
//   vector vec) and push them onto res.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
void clauseset_find_subsumed_clauses_indexed(FVIndex_p index,
                                             Clause_p subsumer,
                                             FreqVector_p vec,
                                             long feature,
                                             PStack_p res,
                                             ClauseSubsumesFun subsumes)
{
   if(feature == vec->size)
   {
      clause_tree_find_subsumed_clauses(index->u1.clauses, subsumer, res, subsumes);
   }
   else if(index->u1.successors)
   {
      long i;
      FVIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors, vec->array[feature], LONG_MAX);

      while((next = IntMapIterNext(iter, &i)))
      {
         if(next->clause_count)
         {
            clauseset_find_subsumed_clauses_indexed(next, subsumer, vec,
                                                    feature+1, res, subsumes);
         }
      }
      IntMapIterFree(iter);
   }
}


/*-----------------------------------------------------------------------
//
// Function: clauseset_find_first_subsumed_clause_indexed()
//
//   Find and return the first clause in the indexed set that is
//   subsumed by subsumer (with feature vector vec).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p clauseset_find_first_subsumed_clause_indexed(FVIndex_p index,
                                                      Clause_p subsumer,
                                                      FreqVector_p vec,
                                                      long feature,
                                                      ClauseSubsumesFun subsumes)
{
   Clause_p res = NULL;

   if(feature == vec->size)
   {
      res = clause_tree_find_first_subsumed_clause(index->u1.clauses,
                                                   subsumer, subsumes);
   }
   else if(index->u1.successors)
   {
      long i;
      FVIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors, vec->array[feature], LONG_MAX);

      while(!res && (next = IntMapIterNext(iter, &i)))
      {
         if(next->clause_count)
         {
            res = clauseset_find_first_subsumed_clause_indexed(next,
                                                               subsumer,
                                                               vec,
                                                               feature+1, subsumes);
         }
      }
      IntMapIterFree(iter);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: clause_tree_find_variant_clause()
//
//   Given a set of clauses and a clause, return the first variant
//   clause (in the order of the set) or NULL
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static
Clause_p clause_tree_find_variant_clause(OrdSet_p tree, Clause_p cand)
{
   OrdSet_p handle;
   Clause_p clause;

   assert(cand->weight == ClauseStandardWeight(cand));

   for(handle = OrdSetFirst(tree); handle; handle = OrdSetNext(tree, handle))
   {
      clause = handle->key;
      if(clause_subsumes_clause(clause,cand) &&
         clause_subsumes_clause(cand, clause))
      {
         return clause;
      }
   }
   return NULL;
}




/*-----------------------------------------------------------------------
//
// Function: clauseset_find_variant_clause_indexed()
//
//   Find and return a variant of clause (with feature vector vec) in
//   the index (if any such clause exists), return NULL otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p clauseset_find_variant_clause_indexed(FVIndex_p index,
                                               Clause_p clause,
                                               FreqVector_p vec,
                                               long feature)
{
   Clause_p res = NULL;
   FVIndex_p next;

   if(feature == vec->size)
   {
      res = clause_tree_find_variant_clause(index->u1.clauses, clause);
   }
   else if(index->u1.successors)
   {
      next = IntMapGetVal(index->u1.successors, vec->array[feature]);
      if(next && next->clause_count)
      {
         res = clauseset_find_variant_clause_indexed(next, clause, vec,
                                                     feature+1);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ft_find_subsuming_clause()
//
//   Return a clause from the feature tree index that subsumes
//   sub_candidate (with feature vector vec), NULL if there is
//   none. Candidates have vectors that are component-wise less than
//   or equal to vec.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p ft_find_subsuming_clause(FTIndex_p index,
                                  Clause_p sub_candidate,
                                  FreqVector_p vec,
                                  ClauseSubsumesFun subsumes)
{
   Clause_p res = NULL;

   if(!index)
   {
      return NULL;
   }
   if(index->final)
   {
      if(FreqVectorNZLessEqual(index->vec, vec))
      {
         res = clause_tree_find_subsuming_clause(index->u1.clauses,
                                                 sub_candidate, subsumes);
      }
   }
   else
   {
      long i;
      FTIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors, 0,
                             vec->array[index->feature]);
      while(!res && (next = IntMapIterNext(iter, &i)))
      {
         res = ft_find_subsuming_clause(next, sub_candidate, vec, subsumes);
      }
      IntMapIterFree(iter);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ft_find_subsumed_clauses()
//
//   Push all clauses in the feature tree index that are subsumed by
//   subsumer (with feature vector vec) onto res. Candidates have
//   vectors that are component-wise greater than or equal to vec.
//   vec needs its list of non-zero positions.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
void ft_find_subsumed_clauses(FTIndex_p index,
                              Clause_p subsumer,
                              FreqVector_p vec,
                              PStack_p res,
                              ClauseSubsumesFun subsumes)
{
   if(!index)
   {
      return;
   }
   if(index->final)
   {
      if(FreqVectorNZLessEqual(vec, index->vec))
      {
         clause_tree_find_subsumed_clauses(index->u1.clauses, subsumer,
                                           res, subsumes);
      }
   }
   else
   {
      long i;
      FTIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors,
                             vec->array[index->feature], LONG_MAX);
      while((next = IntMapIterNext(iter, &i)))
      {
         ft_find_subsumed_clauses(next, subsumer, vec, res, subsumes);
      }
      IntMapIterFree(iter);
   }
}


/*-----------------------------------------------------------------------
//
// Function: ft_find_first_subsumed_clause()
//
//   Return the first clause in the feature tree index that is
//   subsumed by subsumer (with feature vector vec), or NULL. vec
//   needs its list of non-zero positions.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p ft_find_first_subsumed_clause(FTIndex_p index,
                                       Clause_p subsumer,
                                       FreqVector_p vec,
                                       ClauseSubsumesFun subsumes)
{
   Clause_p res = NULL;

   if(!index)
   {
      return NULL;
   }
   if(index->final)
   {
      if(FreqVectorNZLessEqual(vec, index->vec))
      {
         res = clause_tree_find_first_subsumed_clause(index->u1.clauses,
                                                      subsumer, subsumes);
      }
   }
   else
   {
      long i;
      FTIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors,
                             vec->array[index->feature], LONG_MAX);
      while(!res && (next = IntMapIterNext(iter, &i)))
      {
         res = ft_find_first_subsumed_clause(next, subsumer, vec, subsumes);
      }
      IntMapIterFree(iter);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ft_find_variant_clause()
//
//   Return a variant of clause (with feature vector vec) from the
//   feature tree index, or NULL if there is none.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p ft_find_variant_clause(FTIndex_p index, Clause_p clause,
                                FreqVector_p vec)
{
   while(index && !index->final)
   {
      index = IntMapGetVal(index->u1.successors, vec->array[index->feature]);
   }
   if(index && FreqVectorEqual(index->vec, vec))
   {
      return clause_tree_find_variant_clause(index->u1.clauses, clause);
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: fs_find_subsuming_clause()
//
//   Return a clause from the feature set tree index that subsumes
//   sub_candidate (with feature set fset), NULL if there is
//   none. Candidates have feature values less than or equal to those
//   of fset. If bounds is true (only for multiset subsumption),
//   subtrees whose clauses are all heavier than sub_candidate are
//   skipped.
//
// Global Variables: -
//
// Side Effects    : May update weight bounds in the index
//
/----------------------------------------------------------------------*/

static
Clause_p fs_find_subsuming_clause(FSIndex_p index,
                                  Clause_p sub_candidate,
                                  FeatureSet_p fset,
                                  ClauseSubsumesFun subsumes,
                                  bool bounds)
{
   Clause_p res = NULL;

   if(!index ||
      (bounds && FSIndexMinWeight(index) > sub_candidate->weight))
   {
      return NULL;
   }
   if(index->final)
   {
      FSEntry_p entry;

      for(entry = index->u1.entries; !res && entry; entry = entry->next)
      {
         if(FeatureSetLessEqual(entry->fset, fset))
         {
            res = clause_tree_find_subsuming_clause(entry->clauses,
                                                    sub_candidate, subsumes);
         }
      }
   }
   else
   {
      long i;
      FSIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors, 0,
                             FeatureSetGetVal(fset, index->feature));
      while(!res && (next = IntMapIterNext(iter, &i)))
      {
         res = fs_find_subsuming_clause(next, sub_candidate, fset,
                                        subsumes, bounds);
      }
      IntMapIterFree(iter);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: fs_find_subsumed_clauses()
//
//   Push all clauses in the feature set tree index that are subsumed by
//   subsumer (with feature set fset) onto res. Candidates have
//   feature values greater than or equal to those of fset. If bounds
//   is true (only for multiset subsumption), subtrees whose clauses
//   are all lighter than subsumer are skipped.
//
// Global Variables: -
//
// Side Effects    : May update weight bounds in the index
//
/----------------------------------------------------------------------*/

static
void fs_find_subsumed_clauses(FSIndex_p index,
                              Clause_p subsumer,
                              FeatureSet_p fset,
                              PStack_p res,
                              ClauseSubsumesFun subsumes,
                              bool bounds)
{
   if(!index ||
      (bounds && FSIndexMaxWeight(index) < subsumer->weight))
   {
      return;
   }
   if(index->final)
   {
      FSEntry_p entry;

      for(entry = index->u1.entries; entry; entry = entry->next)
      {
         if(FeatureSetLessEqual(fset, entry->fset))
         {
            clause_tree_find_subsumed_clauses(entry->clauses, subsumer,
                                              res, subsumes);
         }
      }
   }
   else
   {
      long i;
      FSIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors,
                             FeatureSetGetVal(fset, index->feature), LONG_MAX);
      while((next = IntMapIterNext(iter, &i)))
      {
         fs_find_subsumed_clauses(next, subsumer, fset, res, subsumes,
                                  bounds);
      }
      IntMapIterFree(iter);
   }
}


/*-----------------------------------------------------------------------
//
// Function: fs_find_first_subsumed_clause()
//
//   Return the first clause in the feature set tree index that is
//   subsumed by subsumer (with feature set fset), or NULL. bounds is
//   as for fs_find_subsumed_clauses().
//
// Global Variables: -
//
// Side Effects    : May update weight bounds in the index
//
/----------------------------------------------------------------------*/

static
Clause_p fs_find_first_subsumed_clause(FSIndex_p index,
                                       Clause_p subsumer,
                                       FeatureSet_p fset,
                                       ClauseSubsumesFun subsumes,
                                       bool bounds)
{
   Clause_p res = NULL;

   if(!index ||
      (bounds && FSIndexMaxWeight(index) < subsumer->weight))
   {
      return NULL;
   }
   if(index->final)
   {
      FSEntry_p entry;

      for(entry = index->u1.entries; !res && entry; entry = entry->next)
      {
         if(FeatureSetLessEqual(fset, entry->fset))
         {
            res = clause_tree_find_first_subsumed_clause(entry->clauses,
                                                         subsumer, subsumes);
         }
      }
   }
   else
   {
      long i;
      FSIndex_p next;
      IntMapIter_p iter;

      iter = IntMapIterAlloc(index->u1.successors,
                             FeatureSetGetVal(fset, index->feature), LONG_MAX);
      while(!res && (next = IntMapIterNext(iter, &i)))
      {
         res = fs_find_first_subsumed_clause(next, subsumer, fset,
                                             subsumes, bounds);
      }
      IntMapIterFree(iter);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: fs_find_variant_clause()
//
//   Return a variant of clause (with feature set fset) from the
//   feature set tree index, or NULL if there is none.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p fs_find_variant_clause(FSIndex_p index, Clause_p clause,
                                FeatureSet_p fset)
{
   FSEntry_p entry;

   while(index && !index->final)
   {
      index = IntMapGetVal(index->u1.successors,
                           FeatureSetGetVal(fset, index->feature));
   }
   if(index)
   {
      for(entry = index->u1.entries; entry; entry = entry->next)
      {
         if(FeatureSetEqual(entry->fset, fset))
         {
            return clause_tree_find_variant_clause(entry->clauses, clause);
         }
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: fb_find_subsuming_clause()
//
//   As fs_find_subsuming_clause(), for the binary feature set
//   tree. Candidates have feature values less than or equal to those
//   of fset, so the successor for values above the threshold is only
//   searched if the value of fset is above it.
//
// Global Variables: -
//
// Side Effects    : May update weight bounds in the index
//
/----------------------------------------------------------------------*/

static
Clause_p fb_find_subsuming_clause(FBIndex_p index,
                                  Clause_p sub_candidate,
                                  FeatureSet_p fset,
                                  ClauseSubsumesFun subsumes,
                                  bool bounds)
{
   Clause_p res = NULL;

   while(index && !res)
   {
      if(bounds && FBIndexMinWeight(index) > sub_candidate->weight)
      {
         return NULL;
      }
      if(index->final)
      {
         FSEntry_p entry;

         for(entry = index->u1.entries; !res && entry; entry = entry->next)
         {
            if(FeatureSetLessEqual(entry->fset, fset))
            {
               res = clause_tree_find_subsuming_clause(entry->clauses,
                                                       sub_candidate,
                                                       subsumes);
            }
         }
         return res;
      }
      res = fb_find_subsuming_clause(index->u1.succ.le, sub_candidate,
                                     fset, subsumes, bounds);
      if(FeatureSetGetVal(fset, index->feature) <= index->threshold)
      {
         return res;
      }
      index = index->u1.succ.gt;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: fb_find_subsumed_clauses()
//
//   As fs_find_subsumed_clauses(), for the binary feature set
//   tree. Candidates have feature values greater than or equal to
//   those of fset, so the successor for values up to the threshold is
//   only searched if the value of fset is at most the threshold.
//
// Global Variables: -
//
// Side Effects    : May update weight bounds in the index
//
/----------------------------------------------------------------------*/

static
void fb_find_subsumed_clauses(FBIndex_p index,
                              Clause_p subsumer,
                              FeatureSet_p fset,
                              PStack_p res,
                              ClauseSubsumesFun subsumes,
                              bool bounds)
{
   while(index)
   {
      if(bounds && FBIndexMaxWeight(index) < subsumer->weight)
      {
         return;
      }
      if(index->final)
      {
         FSEntry_p entry;

         for(entry = index->u1.entries; entry; entry = entry->next)
         {
            if(FeatureSetLessEqual(fset, entry->fset))
            {
               clause_tree_find_subsumed_clauses(entry->clauses, subsumer,
                                                 res, subsumes);
            }
         }
         return;
      }
      if(FeatureSetGetVal(fset, index->feature) <= index->threshold)
      {
         fb_find_subsumed_clauses(index->u1.succ.le, subsumer, fset, res,
                                  subsumes, bounds);
      }
      index = index->u1.succ.gt;
   }
}


/*-----------------------------------------------------------------------
//
// Function: fb_find_first_subsumed_clause()
//
//   As fs_find_first_subsumed_clause(), for the binary feature set
//   tree (see fb_find_subsumed_clauses()).
//
// Global Variables: -
//
// Side Effects    : May update weight bounds in the index
//
/----------------------------------------------------------------------*/

static
Clause_p fb_find_first_subsumed_clause(FBIndex_p index,
                                       Clause_p subsumer,
                                       FeatureSet_p fset,
                                       ClauseSubsumesFun subsumes,
                                       bool bounds)
{
   Clause_p res = NULL;

   while(index && !res)
   {
      if(bounds && FBIndexMaxWeight(index) < subsumer->weight)
      {
         return NULL;
      }
      if(index->final)
      {
         FSEntry_p entry;

         for(entry = index->u1.entries; !res && entry; entry = entry->next)
         {
            if(FeatureSetLessEqual(fset, entry->fset))
            {
               res = clause_tree_find_first_subsumed_clause(entry->clauses,
                                                            subsumer,
                                                            subsumes);
            }
         }
         return res;
      }
      if(FeatureSetGetVal(fset, index->feature) <= index->threshold)
      {
         res = fb_find_first_subsumed_clause(index->u1.succ.le, subsumer,
                                             fset, subsumes, bounds);
      }
      index = index->u1.succ.gt;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: fb_find_variant_clause()
//
//   Return a variant of clause (with feature set fset) from the
//   binary feature set tree index, or NULL if there is none.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static
Clause_p fb_find_variant_clause(FBIndex_p index, Clause_p clause,
                                FeatureSet_p fset)
{
   FSEntry_p entry;

   while(index && !index->final)
   {
      index = (FeatureSetGetVal(fset, index->feature) <= index->threshold)?
         index->u1.succ.le : index->u1.succ.gt;
   }
   if(index)
   {
      for(entry = index->u1.entries; entry; entry = entry->next)
      {
         if(FeatureSetEqual(entry->fset, fset))
         {
            return clause_tree_find_variant_clause(entry->clauses, clause);
         }
      }
   }
   return NULL;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: LiteralSubsumesClause()
//
//   Return true if literal subsumes one of the literals in clause
//   (otherwise return false).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool LiteralSubsumesClause(Eqn_p literal, Clause_p clause)
{
   Eqn_p handle;

   handle = clause->literals;
   while(handle)
   {
      if(EqnIsPositive(literal))
      {
         if(EqnIsPositive(handle) &&
            eqn_subsumes_termpair(literal, handle->lterm,
                                  handle->rterm))
         {
            return true;
         }
      }
      else
      {
         if(EqnIsNegative(handle) &&
            eqn_topsubsumes_termpair(literal, handle->lterm,
                                     handle->rterm))
         {
            return true;
         }
      }
      handle = handle->next;
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: UnitClauseSubsumesClause()
//
//   Return true if unit subsumes clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool UnitClauseSubsumesClause(Clause_p unit, Clause_p clause)
{
   bool res;

   assert(ClauseLiteralNumber(unit) == 1);

   UnitClauseClauseSubsumptionCalls++;
   res = LiteralSubsumesClause(unit->literals, clause);
   //if(res)
   //{
   //   ClauseSetProp(unit, ClauseQueryProp(clause,CPIsSOS));
   //}
   // Not a SOS clause!
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: UnitClauseSetSubsumesClause()
//
//   If a clause in set subsumes clause, return a pointer to
//   it. Otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p UnitClauseSetSubsumesClause(ClauseSet_p set, Clause_p
                                     clause)
{
   Clause_p res;

   res = unit_clause_set_subsumes_clause(set, clause);

   //if(res)
   //{
   //   ClauseSetProp(res, ClauseQueryProp(clause,CPIsSOS));
   // }
   // Not a SOS clause!

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindUnitSubsumedClause()
//
//   Return a pointer to the first clause in the list at or after
//   set_position that is subsumed by the unit-clause subsumer. Return
//   NULL, if no such clause exists.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindUnitSubsumedClause(ClauseSet_p set, Clause_p
                                         set_position, Clause_p
                                         subsumer)
{
   assert(ClauseLiteralNumber(subsumer) == 1);

   while(set_position != set->anchor)
   {
      if(UnitClauseSubsumesClause(subsumer, set_position))
      {
         return set_position;
      }
      set_position = set_position->succ;
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePositiveSimplifyReflect()
//
//   Remove all negative literals subsumed by the positive unit
//   clauses in set from clause. Return true if clause is empty, false
//   otherwise. Set has to be indexed and should contain only positive
//   units! A full proof object keeps the clause before each step in
//   step_archive.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool ClausePositiveSimplifyReflect(ClauseSet_p set, Clause_p clause,
                                   ClauseSet_p step_archive)
{
   Eqn_p   *handle = &(clause->literals);
   ClausePos_p res = NULL;

   while(*handle)
   {
      res = NULL;
      if(!EqnIsPositive(*handle))
      {
         res = StrongUnitForwardSubsumption?
                  unit_clause_set_strongsubsumes_termpair(set,
                                                          (*handle)->lterm,
                                                          (*handle)->rterm,
                                                          true):
                  FindSimplifyingUnit(set,
                                      (*handle)->lterm,
                                      (*handle)->rterm,
                                      true);
      }
      if(!SimplifyFailed(res))
      {
         ClausePos_p pos = res;
         ClauseArchiveStep(step_archive, clause);
         ClauseRemoveLiteralRef(clause, handle);
         if(ClauseQueryProp(pos->clause, CPIsSOS))
         {
            ClauseSetProp(clause, CPIsSOS);
         }
         ClauseDelProp(clause, CPInitial|CPLimitedRW);
         DocClauseModificationDefault(clause, inf_simplify_reflect,
                                      pos->clause);
         ClausePushDerivation(clause, DCSR, pos->clause);
      }
      else
      {
         handle = &((*handle)->next);
      }
   }
   return (clause->literals ==  NULL);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseNegativeSimplifyReflect()
//
//   Remove all positive literals subsumed by negative unit clauses
//   in set from clause. Return true if clause is empty, false
//   otherwise. Set has to be indexed and contain negative units
//   only. A full proof object keeps the clause before each step in
//   step_archive.
//
// Global Variables: -
//
// Side Effects    : Changes clause
//
/----------------------------------------------------------------------*/

bool ClauseNegativeSimplifyReflect(ClauseSet_p set, Clause_p clause,
                                   ClauseSet_p step_archive)
{
   Eqn_p   *handle = &(clause->literals);
   ClausePos_p res = NULL;

   while(*handle)
   {
      res = NULL;
      if(EqnIsPositive(*handle))
      {
         res = FindSignedTopSimplifyingUnit(set,
                                            (*handle)->lterm,
                                            (*handle)->rterm,
                                            false);
      }

      if(!SimplifyFailed(res))
      {
         ClausePos_p pos = res;
         ClauseArchiveStep(step_archive, clause);
         ClauseRemoveLiteralRef(clause, handle);
         if(ClauseQueryProp(pos->clause, CPIsSOS))
         {
            ClauseSetProp(clause, CPIsSOS);
         }
         ClauseDelProp(clause, CPInitial|CPLimitedRW);
         DocClauseModificationDefault(clause, inf_simplify_reflect,
                                      pos->clause);
         ClausePushDerivation(clause, DCSR, pos->clause);
      }
      else
      {
         handle = &((*handle)->next);
      }
   }
   return (clause->literals ==  NULL);
}

/*-----------------------------------------------------------------------
//
// Function: ClauseMultiSetSubsumesClause()
//
//   Return true if subsumer subsumes sub_candidate, where each
//   literal of sub_candidate can be the image of at most one literal
//   of subsumer (multiset subsumption). Requires that both clauses
//   have correct weight information.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool ClauseMultiSetSubsumesClause(Clause_p subsumer, Clause_p sub_candidate)
{
   bool res;

   assert(sub_candidate->weight == ClauseStandardWeight(sub_candidate));
   assert(subsumer->weight == ClauseStandardWeight(subsumer));
   res = clause_subsumes_clause(subsumer, sub_candidate);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePlainSubsumesClauseExcept()
//
//   Return true if there is a substitution that maps every literal of
//   subsumer onto a literal of sub_candidate other than except (which
//   may be NULL). Several literals of subsumer may be mapped onto the
//   same literal (plain or set subsumption, as opposed to multiset
//   subsumption). Without except, a unit subsumer is tested with
//   UnitClauseSubsumesClause(), as in multiset subsumption.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool ClausePlainSubsumesClauseExcept(Clause_p subsumer,
                                     Clause_p sub_candidate,
                                     Eqn_p except)
{
   Subst_p subst;
   bool    res;

   if(ClauseLiteralNumber(subsumer)==0)
   {
      return true;
   }
   if(!except && ClauseLiteralNumber(subsumer)==1)
   {
      return UnitClauseSubsumesClause(subsumer, sub_candidate);
   }
   subst = SubstAlloc();
   res = eqn_list_rec_plain_subsume(subsumer->literals,
                                    sub_candidate->literals,
                                    except, subst);
   SubstDelete(subst);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClausePlainSubsumesClause()
//
//   Return true if subsumer subsumes sub_candidate, where several
//   literals of subsumer may be mapped onto the same literal of
//   sub_candidate (plain or set subsumption).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool ClausePlainSubsumesClause(Clause_p subsumer, Clause_p sub_candidate)
{
   return ClausePlainSubsumesClauseExcept(subsumer, sub_candidate, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: clause_set_subsumes_clause_gen()
//
//   Return a clause from set that subsumes sub_candidate according
//   to subsumes, NULL if there is none. If the set is FV-indexed, the
//   feature vector of sub_candidate is used (or a temporary one
//   computed if it has none). multiset states that subsumes is
//   multiset subsumption (which allows weight bounds). All clauses
//   need correct weights!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static Clause_p clause_set_subsumes_clause_gen(ClauseSet_p set,
                                               Clause_p sub_candidate,
                                               ClauseSubsumesFun subsumes,
                                               bool multiset)
{
   Clause_p res;

   PERF_CTR_ENTRY(SetSubsumeTimer);
   assert(sub_candidate->weight == ClauseStandardWeight(sub_candidate));
   if(set->fvindex && FVIUsesFeatureSets(set->fvindex->structure))
   {
      bool is_temp;
      bool bounds = multiset && set->fvindex->weight_bounds;
      FeatureSet_p fset = ClauseGetFeatureSet(sub_candidate, set->fvindex, &is_temp);

      if(set->fvindex->structure == FVISetTree)
      {
         res = fs_find_subsuming_clause(set->fvindex->fsindex,
                                        sub_candidate, fset, subsumes,
                                        bounds);
      }
      else
      {
         res = fb_find_subsuming_clause(set->fvindex->fbindex,
                                        sub_candidate, fset, subsumes,
                                        bounds);
      }
      if(is_temp)
      {
         FeatureSetFree(fset);
      }
   }
   else if(set->fvindex)
   {
      bool is_temp;
      FreqVector_p vec = ClauseGetFeatureVector(sub_candidate,
                                                set->fvindex, &is_temp);

      if(set->fvindex->structure == FVITree)
      {
         res = ft_find_subsuming_clause(set->fvindex->ftindex,
                                        sub_candidate, vec, subsumes);
      }
      else
      {
         res = clause_set_subsumes_clause_indexed(set->fvindex->index,
                                                  sub_candidate, vec, 0,
                                                  subsumes);
      }
      if(is_temp)
      {
         FreqVectorFree(vec);
      }
   }
   else
   {
      res = clause_set_subsumes_clause(set, sub_candidate, subsumes);
   }
   PERF_CTR_EXIT(SetSubsumeTimer);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetMultiSetSubsumesClause()
//
//   Return a clause from set that (multiset-)subsumes sub_candidate,
//   NULL if there is none. All clauses need correct weights!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetMultiSetSubsumesClause(ClauseSet_p set,
                                         Clause_p sub_candidate)
{
   return clause_set_subsumes_clause_gen(set, sub_candidate,
                                         clause_subsumes_clause, true);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetPlainSubsumesClause()
//
//   Return a clause from set that plain-subsumes sub_candidate, NULL
//   if there is none. With an FV-indexed set, some subsuming clauses
//   may be missed (see above). All clauses need correct weights!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetPlainSubsumesClause(ClauseSet_p set,
                                      Clause_p sub_candidate)
{
   return clause_set_subsumes_clause_gen(set, sub_candidate,
                                         ClausePlainSubsumesClause, false);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindMultiSetSubsumedClause()
//
//   Return a pointer to the first clause in the list at or after
//   set_position that is (multiset-)subsumed by the (non-unit)
//   clause subsumer. Return NULL, if no such clause exists.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindMultiSetSubsumedClause(ClauseSet_p set,
                                             Clause_p set_position,
                                             Clause_p subsumer)
{
   PERF_CTR_ENTRY(SetSubsumeTimer);
   assert(subsumer->weight == ClauseStandardWeight(subsumer));

   while(set_position != set->anchor)
   {
      assert(set_position->weight == ClauseStandardWeight(set_position));
      if(clause_subsumes_clause(subsumer, set_position))
      {
         PERF_CTR_EXIT(SetSubsumeTimer);
         return set_position;
      }
      set_position = set_position->succ;
   }
   PERF_CTR_EXIT(SetSubsumeTimer);
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: clauseset_find_subsumed_clauses_gen()
//
//   Find all clauses in set that are subsumed by subsumer according
//   to subsumes, and push them onto stack. Return number of clauses
//   found. If the set is FV-indexed, the feature vector of subsumer
//   is used (or a temporary one computed if it has none). multiset
//   is as for clause_set_subsumes_clause_gen().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static long clauseset_find_subsumed_clauses_gen(ClauseSet_p set,
                                                Clause_p subsumer,
                                                PStack_p res,
                                                ClauseSubsumesFun subsumes,
                                                bool multiset)
{
   long old_sp = PStackGetSP(res);

   PERF_CTR_ENTRY(SetSubsumeTimer);
   assert(subsumer->weight == ClauseStandardWeight(subsumer));

   if(set->fvindex && FVIUsesFeatureSets(set->fvindex->structure))
   {
      bool is_temp;
      bool bounds = multiset && set->fvindex->weight_bounds;
      FeatureSet_p fset = ClauseGetFeatureSet(subsumer, set->fvindex, &is_temp);

      if(set->fvindex->structure == FVISetTree)
      {
         fs_find_subsumed_clauses(set->fvindex->fsindex,
                                  subsumer, fset, res, subsumes, bounds);
      }
      else
      {
         fb_find_subsumed_clauses(set->fvindex->fbindex,
                                  subsumer, fset, res, subsumes, bounds);
      }
      if(is_temp)
      {
         FeatureSetFree(fset);
      }
   }
   else if(set->fvindex)
   {
      bool is_temp;
      FreqVector_p vec = ClauseGetFeatureVector(subsumer, set->fvindex,
                                                &is_temp);

      if(set->fvindex->structure == FVITree)
      {
         FreqVectorComputeNZ(vec);
         ft_find_subsumed_clauses(set->fvindex->ftindex,
                                  subsumer, vec, res, subsumes);
      }
      else
      {
         clauseset_find_subsumed_clauses_indexed(set->fvindex->index,
                                                 subsumer, vec, 0, res,
                                                 subsumes);
      }
      if(is_temp)
      {
         FreqVectorFree(vec);
      }
   }
   else
   {
      clauseset_find_subsumed_clauses(set, subsumer, res, subsumes);
   }
   PERF_CTR_EXIT(SetSubsumeTimer);
   return PStackGetSP(res)-old_sp;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindMultiSetSubsumedClauses()
//
//   Find all clauses in set that are (multiset-)subsumed by subsumer,
//   and push them onto stack. Return number of clauses found.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetFindMultiSetSubsumedClauses(ClauseSet_p set,
                                          Clause_p subsumer,
                                          PStack_p res)
{
   return clauseset_find_subsumed_clauses_gen(set, subsumer, res,
                                              clause_subsumes_clause, true);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindPlainSubsumedClauses()
//
//   Find all clauses in set that are plain-subsumed by subsumer, and
//   push them onto stack. Return number of clauses found. With an
//   FV-indexed set, some subsumed clauses may be missed (see above).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetFindPlainSubsumedClauses(ClauseSet_p set,
                                       Clause_p subsumer,
                                       PStack_p res)
{
   return clauseset_find_subsumed_clauses_gen(set, subsumer, res,
                                              ClausePlainSubsumesClause,
                                              false);
}


/*-----------------------------------------------------------------------
//
// Function: clauseset_find_first_subsumed_clause_gen()
//
//   Find and return the first clause in set subsumed by subsumer
//   according to subsumes (or NULL). If the set is FV-indexed, the
//   feature vector of subsumer is used (or a temporary one computed
//   if it has none). multiset is as for
//   clause_set_subsumes_clause_gen().
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static Clause_p clauseset_find_first_subsumed_clause_gen(ClauseSet_p set,
                                                        Clause_p subsumer,
                                                        ClauseSubsumesFun subsumes,
                                                        bool multiset)
{
   Clause_p res;

   PERF_CTR_ENTRY(SetSubsumeTimer);
   assert(subsumer->weight == ClauseStandardWeight(subsumer));

   if(set->fvindex && FVIUsesFeatureSets(set->fvindex->structure))
   {
      bool is_temp;
      bool bounds = multiset && set->fvindex->weight_bounds;
      FeatureSet_p fset = ClauseGetFeatureSet(subsumer, set->fvindex, &is_temp);

      if(set->fvindex->structure == FVISetTree)
      {
         res = fs_find_first_subsumed_clause(set->fvindex->fsindex,
                                             subsumer, fset, subsumes,
                                             bounds);
      }
      else
      {
         res = fb_find_first_subsumed_clause(set->fvindex->fbindex,
                                             subsumer, fset, subsumes,
                                             bounds);
      }
      if(is_temp)
      {
         FeatureSetFree(fset);
      }
   }
   else if(set->fvindex)
   {
      bool is_temp;
      FreqVector_p vec = ClauseGetFeatureVector(subsumer, set->fvindex,
                                                &is_temp);

      if(set->fvindex->structure == FVITree)
      {
         FreqVectorComputeNZ(vec);
         res = ft_find_first_subsumed_clause(set->fvindex->ftindex,
                                             subsumer, vec, subsumes);
      }
      else
      {
         res = clauseset_find_first_subsumed_clause_indexed(set->fvindex->index,
                                                            subsumer, vec, 0,
                                                            subsumes);
      }
      if(is_temp)
      {
         FreqVectorFree(vec);
      }
   }
   else
   {
      res = clauseset_find_first_subsumed_clause(set, subsumer, subsumes);
   }
   PERF_CTR_EXIT(SetSubsumeTimer);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindFirstMultiSetSubsumedClause()
//
//   Find and return the first clause in set (multiset-)subsumed by
//   subsumer (or NULL).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindFirstMultiSetSubsumedClause(ClauseSet_p set,
                                                  Clause_p subsumer)
{
   return clauseset_find_first_subsumed_clause_gen(set, subsumer,
                                                   clause_subsumes_clause,
                                                   true);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindFirstPlainSubsumedClause()
//
//   Find and return the first clause in set plain-subsumed by
//   subsumer (or NULL). With an FV-indexed set, some subsumed clauses
//   may be missed (see above).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindFirstPlainSubsumedClause(ClauseSet_p set,
                                               Clause_p subsumer)
{
   return clauseset_find_first_subsumed_clause_gen(set, subsumer,
                                                   ClausePlainSubsumesClause,
                                                   false);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetFindVariantClause()
//
//   Find and return a variant of clause in the FV-indexed set (or
//   NULL if none exists). The feature vector of clause is used (or a
//   temporary one computed if it has none).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p ClauseSetFindVariantClause(ClauseSet_p set,
                                    Clause_p clause)
{
   Clause_p res;
   bool is_temp;
   FreqVector_p vec;

   assert(set->fvindex);
   assert(clause->weight == ClauseStandardWeight(clause));

   if(FVIUsesFeatureSets(set->fvindex->structure))
   {
      FeatureSet_p fset = ClauseGetFeatureSet(clause, set->fvindex, &is_temp);

      res = (set->fvindex->structure == FVISetTree)?
         fs_find_variant_clause(set->fvindex->fsindex, clause, fset) :
         fb_find_variant_clause(set->fvindex->fbindex, clause, fset);
      if(is_temp)
      {
         FeatureSetFree(fset);
      }
      return res;
   }
   vec = ClauseGetFeatureVector(clause, set->fvindex, &is_temp);
   if(set->fvindex->structure == FVITree)
   {
      res = ft_find_variant_clause(set->fvindex->ftindex, clause, vec);
   }
   else
   {
      res = clauseset_find_variant_clause_indexed(set->fvindex->index,
                                                  clause, vec, 0);
   }
   if(is_temp)
   {
      FreqVectorFree(vec);
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
