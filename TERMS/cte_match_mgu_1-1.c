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
#include "cte_pattern_match_mgu.h"
#include <cte_lambda.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#ifdef MEASURE_UNIFICATION
long UnifAttempts  = 0;
long UnifSuccesses = 0;
#endif

PERF_CTR_DEFINE(MguTimer);

#define MATCH_SUCC 0

const UnificationResult UNIF_FAILED = false;
const UnificationResult UNIF_SUCC = true;

#define FAIL_AND_BREAK(res, val) { (res) = (val); break; }

//#undef ENABLE_MATCHING_OPTIMIZATION

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: reorientation_needed()
//
//   Determines whether terms have to be reoriented in HO unification
//   algorithm. Generalizes FO reorientation (rhs var, lhs non-var).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
static __inline__ bool reorientation_needed(Term_p t1, Term_p t2)
{
   return (TermIsTopLevelFreeVar(t2) && !TermIsTopLevelFreeVar(t1))
          || (TermIsTopLevelFreeVar(t1) && TermIsTopLevelFreeVar(t2)
              && t1->arity > t2->arity);
}


/*-----------------------------------------------------------------------
//
// Function: OccurCheck()
//
//   Occur check for variables, possibly more efficient than the
//   general TermIsSubterm()
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OccurCheck(restrict Term_p term, restrict Term_p var)
{
   term = TermDerefAlways(term);

   if(UNLIKELY(term == var))
   {
      return true;
   }

   for(int i=0; i < term->arity; i++)
   {
      if(OccurCheck(term->args[i], var))
      {
         return true;
      }
   }

   return false;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PartiallyMatchVar()
//
//   Given a variable var_matcher, determine the number of arguments
//   of to_match that are actually matched. Performs occur check if
//   needed.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int PartiallyMatchVar(Term_p var_matcher, Term_p to_match, Sig_p sig,
                      bool perform_occur_check)
{
   assert(TermIsFreeVar(var_matcher) && !var_matcher->binding);
   assert(problemType == PROBLEM_HO || !TypeIsArrow(var_matcher->type));
   assert(!TermIsLambda(to_match));

   int args_to_eat = MATCH_FAILED;
   Type_p term_head_type = GetHeadType(sig, to_match);
   Type_p matcher_type   = var_matcher->type;

   if(!term_head_type || TermIsTopLevelDBVar(to_match))
   {
      // ad-hoc polymorphic type -- at the moment we cannot
      // determine these types :(
      return MATCH_FAILED;
   }


   if(matcher_type == to_match->type)
   {
      args_to_eat = ARG_NUM(to_match);
   }
   else if(TypeIsArrow(term_head_type) && TypeIsArrow(matcher_type)
               && matcher_type->arity <= term_head_type->arity)
   {
      int start_idx = term_head_type->arity - matcher_type->arity;

      for(int i=start_idx; i<term_head_type->arity; i++)
      {
         if(matcher_type->args[i-start_idx] != term_head_type->args[i])
         {
            return MATCH_FAILED;
         }
      }

      args_to_eat = start_idx;
      // if they have the same nr of args and args match -> they're the same
      // -> nice place to check the type sharing invariant
      assert(args_to_eat != 0 || matcher_type == term_head_type);
   }

   /* The case where we could eat up arguments, but they are not there. */
   if(args_to_eat > ARG_NUM(to_match))
   {
      return MATCH_FAILED;
   }

   for(int i=0; i<args_to_eat + TermIsAppliedAnyVar(to_match) ? 1 : 0; i++)
   {
      if(!TermIsDBClosed(to_match->args[i]) ||
         (perform_occur_check && OccurCheck(to_match->args[i], var_matcher)))
      {
         return MATCH_FAILED;
      }
   }
   // the number of arguments eaten
   return args_to_eat;
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
   assert(problemType == PROBLEM_FO);
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);

   assert(TermStandardWeight(matcher)  == TermWeight(matcher, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == TermWeight(to_match, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

   if((matcher_weight > to_match_weight) || (TermCellQueryProp(to_match, TPPredPos) && TermIsFreeVar(matcher)))
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

      if(TermIsFreeVar(matcher))
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
// Function: SubstComputeMatchHO()
//
//  Generalization of SubstComputeMatch(). Behaves exactly the same,
//  except for the fact that it matches HO terms and can match prefix of
//  to_match. For details, see
//  SubstComputeMatch().
//
// Global Variables: -
//
// Side Effects    : Instantiates terms
//
/----------------------------------------------------------------------*/
int SubstComputeMatchHO(Term_p matcher, Term_p to_match, Subst_p subst)
{
   assert(problemType == PROBLEM_HO);
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);
   TB_p bank = TermGetBank(matcher) ? TermGetBank(matcher) : TermGetBank(to_match);
   assert(bank || TermIsAnyVar(matcher) || TermIsAnyVar(to_match));
   assert(!(TermGetBank(matcher) && TermGetBank(to_match))
            || TermGetBank(matcher) == TermGetBank(to_match));

   assert(TermStandardWeight(matcher)  == TermWeight(matcher, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == TermWeight(to_match, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

#ifndef NDEBUG
   Term_p s = matcher;
   Term_p t = to_match;
#endif

   int res = MATCH_SUCC;
   if(matcher_weight > to_match_weight ||
      matcher->type != to_match->type)
   {
      return MATCH_FAILED;
   }

   Sig_p sig = bank ? bank->sig : NULL;
   PStackPointer backtrack = PStackGetSP(subst);
   PLocalStackInit(jobs);

   PLocalStackPush(jobs, matcher);
   PLocalStackPush(jobs, to_match);

   // Index from which to start slicing the target term
   int start_idx;

   while(!PLocalStackEmpty(jobs))
   {
      to_match =  PLocalStackPop(jobs);
      matcher  =  PLocalStackPop(jobs);
      PruneLambdaPrefix(bank, &matcher, &to_match);

      if(TermIsTopLevelFreeVar(matcher))
      {
         Term_p var = TermIsAppliedFreeVar(matcher) ? matcher->args[0] : matcher;

         if(var->binding)
         {
            // lambdas can occur if before this algorithm pattern unification has been
            // called. Because of complications with E's dereferencing mechanism of
            // mathching variables -- we currently do no support binding terms
            // to lambdas in this algorithm.
            if(!TermIsLambda(var->binding) && TermIsPrefix(var->binding, to_match))
            {
               start_idx = ARG_NUM(var->binding);
               matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
               if(matcher_weight > to_match_weight)
               {
                  FAIL_AND_BREAK(res, MATCH_FAILED);
               }

               assert(ARG_NUM(to_match) - start_idx - ARG_NUM(matcher) == 0);
            }
            else
            {
               FAIL_AND_BREAK(res, MATCH_FAILED);
            }
         }
         else
         {
            int args_eaten = PartiallyMatchVar(var, to_match, sig, false);
            if(args_eaten == MATCH_FAILED)
            {
               FAIL_AND_BREAK(res, MATCH_FAILED);
            }
            else
            {
               SubstBindAppVar(subst, var, to_match, args_eaten, bank);
               start_idx = args_eaten;

               matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
               if(matcher_weight > to_match_weight)
               {
                  FAIL_AND_BREAK(res, MATCH_FAILED);
               }

               assert(args_eaten + ARG_NUM(matcher) == ARG_NUM(to_match));
            }
         }
      }
      else
      {
         if(TermIsDBVar(matcher) != TermIsDBVar(to_match)
            || TermIsAppliedDBVar(matcher) != TermIsAppliedDBVar(to_match)
            || matcher->arity != to_match->arity)
         {
            FAIL_AND_BREAK(res, MATCH_FAILED);
         }


         if(matcher->f_code != to_match->f_code ||
            (!TermIsTopLevelDBVar(matcher)
              && SigIsPolymorphic(bank->sig, matcher->f_code)
              && matcher->arity != 0
              && matcher->args[0]->type != to_match->args[0]->type))
         {
            FAIL_AND_BREAK(res, MATCH_FAILED);
         }
         else
         {
            assert(ARG_NUM(matcher) == ARG_NUM(to_match));

            start_idx = 0;
         }
      }

      PLocalStackEnsureSpace(jobs, 2*(matcher->arity-(TermIsAppliedFreeVar(matcher)?1:0)));
      for(int i=0; i<matcher->arity-(TermIsAppliedFreeVar(matcher)?1:0); i++)
      {
         PLocalStackPush(jobs, matcher->args[i+(TermIsAppliedFreeVar(matcher)?1:0)]);
         PLocalStackPush(jobs, to_match->args[i+start_idx+(TermIsAppliedFreeVar(to_match)?1:0)]);
      }
   }

   if(res == MATCH_FAILED)
   {
      SubstBacktrackToPos(subst, backtrack);
   }

   PLocalStackFree(jobs);
   assert(res == MATCH_FAILED ||
            TermStructPrefixEqual(s, t, DEREF_ONCE, DEREF_NEVER, res, sig));
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
bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst)
{
   //printf("Unify %lu %lu\n", t1->entry_no, t2->entry_no);
   assert(problemType == PROBLEM_FO);
   #ifdef MEASURE_UNIFICATION
      UnifAttempts++;
   #endif

   PERF_CTR_ENTRY(MguTimer);

   if((TermCellQueryProp(t1, TPPredPos) && TermIsFreeVar(t2))||
      (TermCellQueryProp(t2, TPPredPos) && TermIsFreeVar(t1)))
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
      t2 =  TermDerefAlways(PQueueGetLastP(jobs));
      t1 =  TermDerefAlways(PQueueGetLastP(jobs));

      if(TermIsFreeVar(t2))
      {
         SWAP(t1, t2);
      }

      if(TermIsFreeVar(t1))
      {
         if(t1 != t2)
         {
            assert(t1->type);
            assert(t2->type);
            /* Sort check and occur check - remember, variables are elementary and shared! */
            if((t1->type != t2->type) || OccurCheck(t2, t1))
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
               if(TermIsFreeVar(t1->args[i]) || TermIsFreeVar(t2->args[i]))
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


/*-----------------------------------------------------------------------
//
// Function: SubstComputeMguHO()
//
//  Generalization of SubstComputeMgu(). Behaves exactly the same,
//  except for the fact that it unifies HO terms and can unify a prefix of
//  either t1 or t2. The number of (possible) remaining arguments is stored
//  in UnificationResult.  For other details, see  SubstComputeMgu().
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

UnificationResult SubstComputeMguHO(Term_p t1, Term_p t2, Subst_p subst)
{
   #ifdef MEASURE_UNIFICATION
      UnifAttempts++;
   #endif
   PERF_CTR_ENTRY(MguTimer);
   assert(problemType == PROBLEM_HO);
   if(t1->type != t2->type)
   {
      return UNIF_FAILED;
   }

   PStackPointer backtrack = PStackGetSP(subst);  //For backtracking

   PQueue_p jobs = PQueueAlloc();
   UnificationResult res = UNIF_SUCC;
   TB_p bank = TermGetBank(t1);
   if(!bank)
   {
      bank = TermGetBank(t2);
   }
   Sig_p sig = bank->sig;

   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);

#ifndef NDEBUG
   Term_p debug_t1 = t1;
   Term_p debug_t2 = t2;
#endif

   while(!PQueueEmpty(jobs))
   {
      t2 =  WHNF_deref(PQueueGetLastP(jobs));
      t1 =  WHNF_deref(PQueueGetLastP(jobs));
      // trying to deal with cases X = LAM xyz. complicated term,
      // which cannot be dealt with otherwise.
      if(TermIsFreeVar(t1) && TermIsDBClosed(t2) && (!OccurCheck(t2, t1)))
      {
         SubstAddBinding(subst, t1, t2);
         continue;
      }
      else if(TermIsFreeVar(t2) && TermIsDBClosed(t1) && (!OccurCheck(t1, t2)))
      {
         SubstAddBinding(subst, t2, t1);
         continue;
      }

      PruneLambdaPrefix(bank, &t1, &t2);

      int args_eaten;

      if(reorientation_needed(t1, t2))
      {
         SWAP(t1, t2);
      }

      if(TermIsTopLevelFreeVar(t1))
      {
         Term_p var = TermIsAppliedFreeVar(t1) ? t1->args[0] : t1;
         assert(!TermIsTopLevelFreeVar(t2) || t1->arity <= t2->arity);

         args_eaten = PartiallyMatchVar(var, t2, sig, true);
         if(args_eaten == MATCH_FAILED)
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }

         SubstBindAppVar(subst, var, t2, args_eaten, bank);

         if(var->binding == var)
         {
            var->binding = NULL;
            args_eaten = 0;
            PStackPop(subst);
         }
      }
      else
      {
         args_eaten=0;
         assert(!TermIsTopLevelFreeVar(t1) && !TermIsTopLevelFreeVar(t2));

         if(TermIsDBVar(t1) != TermIsDBVar(t2)
            || TermIsAppliedDBVar(t1) != TermIsAppliedDBVar(t2)
            || t1->arity != t2->arity)
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }

         if(t1->f_code != t2->f_code ||
            (!TermIsTopLevelDBVar(t1)
              && SigIsPolymorphic(bank->sig, t1->f_code)
              && t1->arity != 0
              && t1->args[0]->type != t2->args[0]->type))
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }
      }

      for(int i=0; i<t1->arity-(TermIsAppliedFreeVar(t1)?1:0); i++)
      {
         Term_p arg_t = t1->args[i+(TermIsAppliedFreeVar(t1)?1:0)] ,
                arg_s = t2->args[i+args_eaten+(TermIsAppliedFreeVar(t2)?1:0)];

         if(TermIsTopLevelFreeVar(arg_t) || TermIsTopLevelFreeVar(arg_s))
         {
            PQueueBuryP(jobs, arg_s);
            PQueueBuryP(jobs, arg_t);
         }
         else
         {
            PQueueStoreP(jobs, arg_t);
            PQueueStoreP(jobs, arg_s);
         }
      }
   }

   if(UnifFailed(res))
   {
      SubstBacktrackToPos(subst,backtrack);
   }
   else
   {
      #ifdef MEASURE_UNIFICATION
         UnifSuccesses++;
      #endif

      assert(TermStructPrefixEqual(debug_t1, debug_t2, DEREF_ALWAYS, DEREF_ALWAYS, 0, sig));
   }

   PQueueFree(jobs);

   PERF_CTR_EXIT(MguTimer);
   return res;
}


// Definitions are needed only if we are working in LFHOL mode.
// Otherwise, macros expand to usual FO functions.
#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: SubstMatchComplete()
//
//  Determines whether pattern matches target so that no arguments remain
//  in the target. If so, it adds bindings to subst and returns true.
//  Otherwise, leaves subst unchanged and returns false.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
__inline__ bool SubstMatchComplete(Term_p pattern, Term_p target, Subst_p subst)
{
   bool res;
   if(problemType == PROBLEM_FO)
   {
      res = SubstComputeMatch(pattern, target, subst);
   }
   else
   {
      PStackPointer backtrack = PStackGetSP(subst);

      Term_p reduced_p = LambdaEtaReduceDB(TermGetBank(pattern), pattern),
             reduced_t = LambdaEtaReduceDB(TermGetBank(target), target);
      int res_i = SubstComputeMatchHO(reduced_p, reduced_t, subst);

      if(res_i != 0 && TermIsNonFOPattern(pattern) && TermIsNonFOPattern(target))
      {
         SubstBacktrackToPos(subst, backtrack);
         res_i = SubstComputeMatchPattern(pattern, target, subst) == UNIFIABLE ?
                 0 : MATCH_FAILED;
         if(res_i != 0)
         {
            SubstBacktrackToPos(subst, backtrack);
         }
      }
      res = res_i == 0;  // are no arguments of s remaining after the match?
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SubstMguComplete()
//
//  Determines whether t unifies with s so that no arguments remain
//  in either t or s. If so, it adds bindings to subst and returns true.
//  Otherwise, leaves subst unchanged and returns false.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
__inline__ bool SubstMguComplete(Term_p t, Term_p s, Subst_p subst)
{
   bool res;
   // DBG_PRINT(stderr, "unifying: ", TermPrintDbg(stderr, t,
   //           TermGetBank(t)->sig, DEREF_NEVER), " <=?=> ");
   // DBG_PRINT(stderr, "", TermPrintDbg(stderr, s,
   //           TermGetBank(s)->sig, DEREF_NEVER), ".\n");
   if(problemType != PROBLEM_HO)
   {
      res = SubstComputeMgu(t, s, subst);
   }
   else
   {
      // no arguments of s remaining after the match
      PStackPointer backtrack = PStackGetSP(subst);
      Term_p reduced_t = LambdaEtaReduceDB(TermGetBank(t), t),
             reduced_s = LambdaEtaReduceDB(TermGetBank(s), s);

      res = SubstComputeMguHO(reduced_t, reduced_s, subst);

      if(UnifFailed(res) && TermIsNonFOPattern(t) && TermIsNonFOPattern(s))
      {
         SubstBacktrackToPos(subst, backtrack);
         OracleUnifResult oracle_res = SubstComputeMguPattern(t, s, subst);
         if(oracle_res != UNIFIABLE)
         {
            SubstBacktrackToPos(subst, backtrack);
         }
         else
         {
            res = true;
         }
      }
   }
   // if(res)
   // {
   //    DBG_PRINT(stderr, "success: ",
   //              SubstPrint(stderr, subst, TermGetBank(t)->sig, DEREF_NEVER), ".\n");
   // }
   // else
   // {
   //    DBG_PRINT(stderr, "failed: ",
   //              SubstPrint(stderr, subst, TermGetBank(t)->sig, DEREF_NEVER), ".\n");
   // }
   return res;
}
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
