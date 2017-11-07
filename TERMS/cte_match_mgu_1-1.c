/*-----------------------------------------------------------------------

File  : cte_match_mgu_1-1.c

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

#include "cte_match_mgu_1-1.h"
#include "clb_plocalstacks.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#ifdef MEASURE_UNIFICATION
long UnifAttempts  = 0;
long UnifSuccesses = 0;
#endif

PERF_CTR_DEFINE(MguTimer);

#define MATCH_INIT -2

const UnificationResult UNIF_FAILED = {NoTerm, -1};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: occur_check()
//
//   Occur check for variables, possibly more efficient than the
//   general TermIsSubterm()
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool occur_check(restrict Term_p term, restrict Term_p var)
{
   term = TermDerefAlways(term);

   if(UNLIKELY(term == var))
   {
      return true;
   }

   for(int i=0; i < term->arity; i++)
   {
      if(occur_check(term->args[i], var))
      {
         return true;
      }
   }

   return false;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

int PartiallyMatchVar(Term_p var_matcher, Term_p to_match, Sig_p sig)
{
   assert(TermIsVar(var_matcher) && !var_matcher->binding);
   assert(!TermIsAppliedVar(to_match) || to_match->f_code == sig->app_var_code);

   int matched_up_to = NOT_MATCHED;
   Type_p term_head_type = GetHeadType(sig, to_match);
   Type_p matcher_type   = var_matcher->type;

   if (matcher_type == to_match->type)
   {
      matched_up_to = to_match->arity - (TermIsAppliedVar(to_match) ? 1 : 0);
   }
   else if (TypeIsArrow(term_head_type) && TypeIsArrow(matcher_type) 
               && matcher_type->arity <= term_head_type->arity)
   {
      int start_idx = term_head_type->arity - matcher_type->arity;

      for(int i=start_idx; i<term_head_type->arity; i++)
      {
         if (matcher_type->args[i-start_idx] != term_head_type->args[i])
         {
            return NOT_MATCHED;
         }
      }

      matched_up_to = start_idx;
      // if they have the same nr of args and args match -> they're the same
      // -> nice place to check the type sharing invariant
      assert(matched_up_to != 0 || matcher_type == term_head_type);
   }

   // the number of arguments eaten
   return matched_up_to;
}

// TODO: add weight computation.
int SubstComputeMatchHO(Term_p matcher, Term_p to_match, Subst_p subst, Sig_p sig)
{
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);

   assert(TermStandardWeight(matcher)  == TermWeight(matcher, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == TermWeight(to_match, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

   if((matcher_weight > to_match_weight) || (TermCellQueryProp(to_match, TPPredPos) && TermIsVar(matcher)))
   {
      return NOT_MATCHED;
   }

   int res = MATCH_INIT;
   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */
   PLocalStackInit(matcher_stack);
   PLocalStackInit(to_match_stack);

   PLocalStackPush(matcher_stack, matcher);
   PLocalStackPush(to_match_stack, to_match);

   while(!PLocalStackEmpty(matcher_stack) && !PLocalStackEmpty(to_match_stack))
   {
      to_match =  PLocalStackPop(to_match_stack);
      matcher  =  PLocalStackPop(matcher_stack);

      /*fprintf(stderr, "$ Matcher : ");
      TermPrint(stderr, matcher, sig, DEREF_NEVER);
      fprintf(stderr, " , to match : ");
      TermPrint(stderr, to_match, sig, DEREF_NEVER);
      fprintf(stderr, "\n");*/

      if (TermIsAppliedVar(matcher) || TermIsVar(matcher))
      {
         Term_p var = TermIsAppliedVar(matcher) ? matcher->args[0] : matcher;

         // if it is bound -> then it has to be bound to prefix.
         if (var->binding && TermIsPrefix(var->binding, to_match))
         {
            int bound_arity = var->binding->arity;
            PLocalStackEnsureSpace(to_match_stack, to_match->arity - bound_arity);

            for(int i=to_match->arity-1; i >= bound_arity; i--)
            {
               PLocalStackPush(to_match_stack, to_match->args[i]);
            }

            if (matcher->arity)
            {
               PLocalStackEnsureSpace(matcher_stack, matcher->arity-1);

               for(int i=matcher->arity-1; i; i--)
               {
                  PLocalStackPush(matcher_stack, matcher->args[i]);  
               }
            }

            matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
            if(matcher_weight > to_match_weight)
            {
               res = NOT_MATCHED;
               break;
            }
         }
         else if (var->binding)
         {
            res = NOT_MATCHED;
            break;
         }
         else
         {
            int matched_up_to = PartiallyMatchVar(var, to_match, sig);
            if (matched_up_to != NOT_MATCHED)
            {
               SubstBindAppVar(subst, var, to_match, matched_up_to);

               matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
               if(matcher_weight > to_match_weight)
               {
                  res = NOT_MATCHED;
                  break;
               }

               if (TermIsAppliedVar(to_match))
               {
                  matched_up_to++;
               }

               /*fprintf(stderr, "$     var  ");
               TermPrint(stderr, var, sig, DEREF_NEVER);
               fprintf(stderr, " bound to ");
               TermPrint(stderr, var->binding, sig, DEREF_NEVER);
               fprintf(stderr, " matched up to %d.\n", matched_up_to);*/

               PLocalStackEnsureSpace(to_match_stack, to_match->arity - matched_up_to);
               for(int i=to_match->arity-1; i >= matched_up_to; i--)
               {
                  PLocalStackPush(to_match_stack, to_match->args[i]);
               }

               if (matcher->arity)
               {
                  PLocalStackEnsureSpace(matcher_stack, matcher->arity-1);

                  for(int i=matcher->arity-1; i; i--)
                  {
                     PLocalStackPush(matcher_stack, matcher->args[i]);  
                  }
               }   
            }
            else
            {
               res = NOT_MATCHED;
               break;
            }   
         }
      }
      else
      {
         if(matcher->f_code != to_match->f_code)
         {
            // if the RHS is applied var -- bad luck.
            res = NOT_MATCHED;
            break;
         }
         else
         {
            PLocalStackEnsureSpace(to_match_stack, to_match->arity);
            for(int i=to_match->arity-1; i >= 0; i--)
            {
               PLocalStackPush(to_match_stack, to_match->args[i]);
            }

            PLocalStackEnsureSpace(matcher_stack, matcher->arity);
            for(int i=matcher->arity-1; i >= 0; i--)
            {
               PLocalStackPush(matcher_stack, matcher->args[i]);
            }
         }
      }
   }

   if (res == MATCH_INIT && PLocalStackEmpty(matcher_stack))
   {
      res = PLocalStackTop(to_match_stack);
   }
   else
   {
      res = NOT_MATCHED;
   }

   if(res == NOT_MATCHED)
   {
      SubstBacktrackToPos(subst,backtrack);
   }

   PLocalStackFree(to_match_stack);
   PLocalStackFree(matcher_stack);
   return res;

}

/*-----------------------------------------------------------------------
//
// Function: SubstComputeMatch()
//
//   Try to compute a match from matcher onto to_match and record it in
//   subst. Return true if match exits (in this case subst is
//   changed and needs to be backtracked by the caller), false
//   otherwise (subst is unchanged). Both terms are assumed to contain
//   no bindings except those stored in subst.
//
//   The routine will work and compute a valid match if the two terms
//   share variables. However, this will lead to temporary incorrect
//   term structures (a variable may be bound to itself or a superterm
//   containing it).
//
// Global Variables: -
//
// Side Effects    : Instantiates terms
//
/----------------------------------------------------------------------*/

bool SubstComputeMatch(Term_p matcher, Term_p to_match, Subst_p subst)
{
   assert(ProblemIsHO == PROBLEM_NOT_HO);
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);

   assert(TermStandardWeight(matcher)  == TermWeight(matcher, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == TermWeight(to_match, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

   if((matcher_weight > to_match_weight) || (TermCellQueryProp(to_match, TPPredPos) && TermIsVar(matcher)))
   {
      return false;
   }

   bool res = true;
   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */
   PLocalStackInit(jobs);

   PLocalStackPush(jobs, matcher);
   PLocalStackPush(jobs, to_match);

   while(!PLocalStackEmpty(jobs))
   {
      to_match =  PLocalStackPop(jobs);
      matcher  =  PLocalStackPop(jobs);

      if(TermIsVar(matcher))
      {
         assert(matcher->type);
         assert(to_match->type);
         if(matcher->type != to_match->type)
         {
            res = false;
            break;
         }
         if(matcher->binding)
         {
            if(matcher->binding != to_match)
            {
               res = false;
               break;
            }
         }
         else
         {
            SubstAddBinding(subst, matcher, to_match);
         }

         matcher_weight += TermStandardWeight(to_match) - DEFAULT_VWEIGHT;

         if(matcher_weight > to_match_weight)
         {
            res = false;
            break;
         }
      }
      else
      {
         if(matcher->f_code != to_match->f_code)
         {
            res = false;
            break;
         }
         else
         {
            PLocalStackEnsureSpace(jobs, 2*matcher->arity);
            for(int i=matcher->arity-1; i>=0; i--)
            {
               PLocalStackPush(jobs, matcher->args[i]);
               PLocalStackPush(jobs, to_match->args[i]);
            }
         }
      }
   }

   PLocalStackFree(jobs);
   if(!res)
   {
      SubstBacktrackToPos(subst,backtrack);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SubstComputeMgu()
//
//   Compute an mgu between two terms. Currently without any special
//   optimization (double entry checking in the  to-solve stack has
//   been deleted as ineficient). Returns true and modifies
//   subst if sucessful, false otherwise (as for match, see
//   above). Terms have to be variable disjoint, otherwise behaviour
//   is unpredictable!
//
//   Solution with stacks is more efficient than unsorted queues,
//   sorted queues (variables last) are significantly better again!
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

static __inline__ bool unify_var(Term_p var, Term_p match_to, PStack_p match_to_stack, Sig_p sig, Subst_p subst)
{
   if (TermIsVar(match_to))
   {
      if (var == match_to)
      {
         return true;
      }
      else if (var->type == match_to->type)
      {
         SubstAddBinding(subst, var, match_to);
         return true;
      }
      return false;
   }
   else
   {
      int res = PartiallyMatchVar(var, match_to, sig);
      if (res != NOT_MATCHED)
      {
         SubstBindAppVar(subst, var, match_to, res);
         res += TermIsAppliedVar(match_to) ? 1 : 0;

         for(int i=match_to->arity-1; i >= res; i--)
         {
            PStackPushP(match_to_stack, match_to->args[i]);
         }
      }
      return res != NOT_MATCHED;
   }
}

static void __inline__ push_rest(Term_p var, PStack_p stack)
{
   for(int i = var->arity-1; i; i--)
   {
      PStackPushP(stack, var->args[i]);
   }
}

void print_stack(PStack_p stack, Sig_p sig)
{
   for(int i=0; i<PStackGetSP(stack); i++)
   {
      Term_p term = PStackElementP(stack, i);
      fprintf(stderr, "%d - ", i);
      TermPrint(stderr, term, sig, DEREF_ALWAYS);
      fprintf(stderr, "\n");
   }
}

UnificationResult SubstComputeMguHO(Term_p t1, Term_p t2, Subst_p subst, Sig_p sig)
{
   //printf("Unify %lu %lu\n", t1->entry_no, t2->entry_no);
   #ifdef MEASURE_UNIFICATION
      UnifAttempts++;
   #endif

   PERF_CTR_ENTRY(MguTimer);

   if((TermCellQueryProp(t1, TPPredPos) && TermIsVar(t2))||
      (TermCellQueryProp(t2, TPPredPos) && TermIsVar(t1)))
   {
      PERF_CTR_EXIT(MguTimer);
      return UNIF_FAILED;

   }
   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */

   bool res = true;
   PStack_p jobs_t1 = PStackAlloc();
   PStack_p jobs_t2 = PStackAlloc();

   PStackPushP(jobs_t1, t1);
   PStackPushP(jobs_t2, t2);

   while(!PStackEmpty(jobs_t1) && !PStackEmpty(jobs_t2))
   {
      t1 = TermDerefAlways(PStackPopP(jobs_t1));
      t2 = TermDerefAlways(PStackPopP(jobs_t2));

      /*fprintf(stderr, "\n? unifiying ");
      TermPrint(stderr, t1, sig, DEREF_NEVER);
      fprintf(stderr, " === ");
      TermPrint(stderr, t2, sig, DEREF_NEVER);
      fprintf(stderr, " . ");*/

      assert(t1->type);
      assert(t2->type);
      
      bool bound = false;
      if(TermIsVar(t1) || TermIsAppliedVar(t1))
      {
         Term_p var = TermIsAppliedVar(t1) ? t1->args[0] : t1;
         bound = unify_var(var, t2, jobs_t2, sig, subst);
         // this one can fail! -- we can reorient variables and try again.
         if(bound && TermIsAppliedVar(t1))
         {
            push_rest(t1, jobs_t1);
         }
      }
      // trying to bind t1 to t2 can fail, but t2 to t1 can succeeed
      // this failure will be very cheap -- 
      if (!bound && (TermIsVar(t2) || TermIsAppliedVar(t2)))
      {         
         Term_p var = TermIsAppliedVar(t2) ? t2->args[0] : t2;
         if (!(bound = unify_var(var, t1, jobs_t1, sig, subst)))
         {
            res = false;
            break;
         }
         if(TermIsAppliedVar(t2))
         {
            push_rest(t2, jobs_t2);
         }
      }

      // the previous did not suceed.
      if(!bound)
      {
         if(t1->f_code != t2->f_code)
         {
            res = false;
            break;
         }
         else
         {
            assert(t1->type);
            assert(t2->type);
            //assert(t1->type == t2->type);

            for(int i=t1->arity-1; i>=0; i--)
            {
               PStackPushP(jobs_t1, t1->args[i]);
            }
            for(int i=t2->arity-1; i>=0; i--)
            {
               PStackPushP(jobs_t2, t2->args[i]);
            }
         }
      }
   }

   UnificationResult un_res = UNIF_FAILED;

   if(!res)
   {
      SubstBacktrackToPos(subst,backtrack);
   }
   else
   {
      #ifdef MEASURE_UNIFICATION
         UnifSuccesses++;
      #endif
      WhichTerm term_side = PStackEmpty(jobs_t1) ? RightTerm : LeftTerm;
      if (term_side == RightTerm)
      {
         un_res = (UnificationResult){term_side, PStackGetSP(jobs_t2)};
      }
      else
      {
         un_res = (UnificationResult){term_side, PStackGetSP(jobs_t1)};
      }
   }

   PStackFree(jobs_t1);
   PStackFree(jobs_t2);

   PERF_CTR_EXIT(MguTimer);
   return un_res;
}


bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst)
{
   //printf("Unify %lu %lu\n", t1->entry_no, t2->entry_no);
   assert(ProblemIsHO == PROBLEM_NOT_HO);
   #ifdef MEASURE_UNIFICATION
      UnifAttempts++;
   #endif

   PERF_CTR_ENTRY(MguTimer);

   if((TermCellQueryProp(t1, TPPredPos) && TermIsVar(t2))||
      (TermCellQueryProp(t2, TPPredPos) && TermIsVar(t1)))
   {
      PERF_CTR_EXIT(MguTimer);
      return false;

   }
   PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */

   bool res = true;
   PQueue_p jobs = PQueueAlloc();

   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);

   while(!PQueueEmpty(jobs))
   {
      // TODO: Terms might not be shared
      t2 =  TermDerefAlways(PQueueGetLastP(jobs));
      t1 =  TermDerefAlways(PQueueGetLastP(jobs));

      if(TermIsVar(t2))
      {
         SWAP(t1, t2);
      }

      if(TermIsVar(t1))
      {
         if(t1 != t2)
         {
            assert(t1->type);
            assert(t2->type);
            /* Sort check and occur check - remember, variables are elementary and shared! */
            if((t1->type != t2->type) || occur_check(t2, t1))
            {
               res = false;
               break;
            }
            else
            {
               SubstAddBinding(subst, t1, t2);
            }
         }
      }
      else
      {
         if(t1->f_code != t2->f_code)
         {
            res = false;
            break;
         }
         else
         {
            assert(t1->type);
            assert(t2->type);
            assert(t1->type == t2->type);
            for(int i=t1->arity-1; i>=0; i--)
            {
               /* Delay variable bindings */
               if(TermIsVar(t1->args[i]) || TermIsVar(t2->args[i]))
               {
                  PQueueBuryP(jobs, t2->args[i]);
                  PQueueBuryP(jobs, t1->args[i]);
               }
               else
               {
                  PQueueStoreP(jobs, t1->args[i]);
                  PQueueStoreP(jobs, t2->args[i]);
               }
            }
         }
      }
   }
   PQueueFree(jobs);

   if(!res)
   {
      SubstBacktrackToPos(subst,backtrack);
   }
   else
   {
      #ifdef MEASURE_UNIFICATION
         UnifSuccesses++;
      #endif
   }

   PERF_CTR_EXIT(MguTimer);
   return res;
}


#ifdef ENABLE_LFHO
__inline__ bool SubstMatchComplete(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      return SubstComputeMatch(t, s, subst);
   }
   else
   {
      // no arguments of s remaining after the match
      PStackPointer backtrack = PStackGetSP(subst);
      int res =  SubstComputeMatchHO(t, s, subst, sig);
      if (res != 0)
      {
         SubstBacktrackToPos(subst, backtrack);
      }
      return res == 0;
   }
}

__inline__ bool SubstMguComplete(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      return SubstComputeMgu(t, s, subst);
   }
   else
   {
      // no arguments of s remaining after the match
      PStackPointer backtrack = PStackGetSP(subst);
      UnificationResult res =  SubstComputeMguHO(t, s, subst, sig);
      if (UnifFailed(res) || res.term_remaining != 0)
      {
         SubstBacktrackToPos(subst, backtrack);
      }
      return !UnifFailed(res) && res.term_remaining == 0;
   }  
}

__inline__ int SubstMatchPossiblyPartial(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      return SubstComputeMatch(t, s, subst) ? 0 : NOT_MATCHED;
   }
   else
   {
      return SubstComputeMatchHO(t, s, subst, sig);
   }
}
#endif

UnificationResult SubstMguPossiblyPartial(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      return (UnificationResult) {SubstComputeMgu(t,s,subst) ? RightTerm : NoTerm, 0};
   }
   else
   {
      return SubstComputeMguHO(t,s,subst,sig);
   }
}
/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


