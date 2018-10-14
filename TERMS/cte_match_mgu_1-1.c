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
const UnificationResult UNIF_INIT = {NoTerm, -2};

#define FAIL_AND_BREAK(res, val) { (res) = (val); break; }
#define UPDATE_IF_INIT(res, new) ((res) = ((res) == MATCH_INIT) ? (new) : (res))

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
   if(TermIsTopLevelVar(t2))
   {
      return !TermIsTopLevelVar(t1) ||
               TypeGetMaxArity(GetHeadType(NULL, t2)) <
               TypeGetMaxArity(GetHeadType(NULL, t1)) ||
               (TypeGetMaxArity(GetHeadType(NULL, t2)) ==
               TypeGetMaxArity(GetHeadType(NULL, t1)) && t2->arity < t1->arity);
   }
   else
   {
      return false;
   }
}


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
   assert(TermIsVar(var_matcher) && !var_matcher->binding);
   assert(problemType == PROBLEM_HO || !TypeIsArrow(var_matcher->type));
   
   int args_to_eat = MATCH_FAILED;
   Type_p term_head_type = GetHeadType(sig, to_match);
   Type_p matcher_type   = var_matcher->type;

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

   if(perform_occur_check)
   {
      for(int i=0; i<args_to_eat + TermIsAppliedVar(to_match) ? 1 : 0; i++)
      {
         if(occur_check(to_match->args[i], var_matcher))
         {
            return MATCH_FAILED;
         }
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
   assert(TermGetBank(matcher) == TermGetBank(to_match));
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);
   TB_p bank = TermGetBank(matcher);

   assert(TermStandardWeight(matcher)  == TermWeight(matcher, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == TermWeight(to_match, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

#ifndef NDEBUG
   Term_p s = matcher;
   Term_p t = to_match;
#endif 

   int res = MATCH_INIT;
   if(matcher_weight > to_match_weight)
   {
      return MATCH_FAILED;
   }

   Sig_p sig = bank->sig;
   
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
      
      if(TermIsTopLevelVar(matcher))
      {
         Term_p var = TermIsAppliedVar(matcher) ? matcher->args[0] : matcher;

         if(var->binding)
         {
            if(TermIsPrefix(var->binding, to_match))
            {
               start_idx = ARG_NUM(var->binding);
               matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
               if(matcher_weight > to_match_weight)
               {
                  FAIL_AND_BREAK(res, MATCH_FAILED);
               }

               assert(ARG_NUM(to_match) - start_idx - ARG_NUM(matcher) == 0 || res == MATCH_INIT);
               UPDATE_IF_INIT(res, ARG_NUM(to_match) - start_idx - ARG_NUM(matcher));
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

               assert(args_eaten + ARG_NUM(matcher) == ARG_NUM(to_match) || res == MATCH_INIT);
               UPDATE_IF_INIT(res, ARG_NUM(to_match) - args_eaten - ARG_NUM(matcher));
            }
         }
      }
      else
      {
         if(matcher->f_code != to_match->f_code 
               || matcher->arity > to_match ->arity)
         {
            FAIL_AND_BREAK(res, MATCH_FAILED);
         }
         else
         {
            assert(ARG_NUM(matcher) == ARG_NUM(to_match) || res == MATCH_INIT);

            start_idx = 0;
            UPDATE_IF_INIT(res, ARG_NUM(to_match) - ARG_NUM(matcher));
         }
      }

      const int offset = start_idx + (TermIsAppliedVar(to_match) ? 1 : 0)
                                   - (TermIsAppliedVar(matcher) ? 1 : 0);
      if(matcher->arity + offset > to_match->arity)
      {
         FAIL_AND_BREAK(res, MATCH_FAILED);
      }
      
      PLocalStackEnsureSpace(jobs, 2*(matcher->arity));
      for(int i=TermIsAppliedVar(matcher) ? 1 : 0; i<matcher->arity; i++)
      {
         PLocalStackPush(jobs, matcher->args[i]);
         PLocalStackPush(jobs, to_match->args[i+offset]);
      }
   }

   if(res == MATCH_FAILED)
   {
      SubstBacktrackToPos(subst, backtrack);
   }

   PLocalStackFree(jobs);
   assert(res != MATCH_INIT);
   assert(res == MATCH_FAILED || 
            TermStructPrefixEqual(s, t, DEREF_ONCE, DEREF_NEVER, res, bank->sig));
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
   assert(TermGetBank(t1) == TermGetBank(t2));

   PStackPointer backtrack = PStackGetSP(subst);  //For backtracking 

   PQueue_p jobs = PQueueAlloc();
   UnificationResult res = UNIF_INIT;
   bool swapped = reorientation_needed(t1, t2);
   TB_p bank = TermGetBank(t1);
   Sig_p sig = bank->sig;

   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);

#ifndef NDEBUG
   Term_p debug_t1 = t1;
   Term_p debug_t2 = t2;
#endif

   while(!PQueueEmpty(jobs))
   {
      //fprintf(stderr, "|jobs|: %ld\n", PQueueCardinality(jobs));
      t2 =  TermDerefAlways(PQueueGetLastP(jobs));
      t1 =  TermDerefAlways(PQueueGetLastP(jobs)); 

      int start_idx;

      if(reorientation_needed(t1, t2))
      {
         SWAP(t1, t2);
      }

      if(TermIsTopLevelVar(t1))
      {
         Term_p var = TermIsAppliedVar(t1) ? t1->args[0] : t1;
         
         int args_eaten = PartiallyMatchVar(var, t2, sig, true);
         if(args_eaten == MATCH_FAILED)
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }
         
         SubstBindAppVar(subst, var, t2, args_eaten, bank);
         
         if(var->binding == var)
         {
            var->binding = NULL;
            start_idx = 0;
            PStackPop(subst);
         }
         else
         {
            start_idx = ARG_NUM(var->binding);

            assert(args_eaten == ARG_NUM(var->binding));   
         }      
      }
      else
      {
         assert(!TermIsTopLevelVar(t1) && !TermIsTopLevelVar(t2));

         if(t1->f_code != t2->f_code)
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }

         start_idx = 0;         
      }

      Term_p min_term = t1;
      Term_p max_term = t2;
      int offset_min = 0;
      int offset_max = start_idx;

      if(ARG_NUM(t1) > ARG_NUM(t2) - start_idx)
      {
         assert(UnifIsInit(res));
         // making sure that the argument with less arguments is on the left
         // previously we made sure that the variable is on the left.
         SWAP(min_term, max_term);
         SWAP(offset_min, offset_max);
      }

      offset_min += (TermIsAppliedVar(min_term) ? 1 : 0);
      offset_max += (TermIsAppliedVar(max_term) ? 1 : 0);
      
      assert(min_term->arity - offset_min <= max_term->arity - offset_max && min_term->arity >= offset_min &&
               max_term->arity >= offset_max);

      if(UnifIsInit(res))
      {
         res = (UnificationResult){min_term == t1 ? (!swapped ? RightTerm : LeftTerm) 
                                                      : (!swapped ? LeftTerm : RightTerm), 
                                  ABS(ARG_NUM(t1) - ARG_NUM(t2) + start_idx)};   
      }

      for(int i=0; i<min_term->arity - offset_min; i++)
      {
         Term_p min_arg = min_term->args[i + offset_min];
         Term_p max_arg = max_term->args[i + offset_max];
         if(TermIsTopLevelVar(min_arg) || TermIsTopLevelVar(max_arg))
         {
            PQueueBuryP(jobs, max_arg);
            PQueueBuryP(jobs, min_arg);
         }
         else
         {
            PQueueStoreP(jobs, min_arg);
            PQueueStoreP(jobs, max_arg);
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

      assert(TermStructPrefixEqual(res.term_side == RightTerm ? debug_t1 : debug_t2,
                                   res.term_side == RightTerm ? debug_t2 : debug_t1,
                                   DEREF_ALWAYS, DEREF_ALWAYS, res.term_remaining,
                                   sig));
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

      int res_i =  SubstComputeMatchHO(pattern, target, subst);

      if(res_i != 0)
      {
         SubstBacktrackToPos(subst, backtrack);
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
   if(problemType == PROBLEM_FO)
   {
      res = SubstComputeMgu(t, s, subst);
   }
   else
   {
      // no arguments of s remaining after the match
      PStackPointer backtrack = PStackGetSP(subst);
      UnificationResult u_res =  SubstComputeMguHO(t, s, subst);

      if(UnifFailed(u_res) || u_res.term_remaining != 0)
      {
         SubstBacktrackToPos(subst, backtrack);
      }
      
      res = !UnifFailed(u_res) && u_res.term_remaining == 0;
   }  

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: SubstMatchPossiblyPartial()
//
//  Determines if pattern can match target so that n arguments are
//  remaining in target (n <= ARG_NUM(target)). In that case, it adds
//  bindings to subst and returns n. Otherwise returns MATCH_FAILED and
//  leaves subst unchanged.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
__inline__ int SubstMatchPossiblyPartial(Term_p pattern, Term_p target, Subst_p subst)
{
   int res;
   if(problemType == PROBLEM_FO)
   {
      res = SubstComputeMatch(pattern, target, subst) ? 0 : MATCH_FAILED;
   }
   else
   {
      res = SubstComputeMatchHO(pattern, target, subst);
   }

   // if matched -> number of remaining args is in good range
   assert(res == MATCH_FAILED|| res <= ARG_NUM(target)); 
   return res;
}
#endif

/*-----------------------------------------------------------------------
//
// Function: SubstMguPossiblyPartial()
//
//  Determines if t can unify with s so that n arguments are
//  remaining in either side (n <= ARG_NUM(t) || n <= ARG_NUM(s)). 
//  In that case, it adds bindings to subst and returns object encoding n 
//  and term in which arguments are remaining. Otherwise returns 
//  object encoding failure and leaves subst unchanged.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
UnificationResult SubstMguPossiblyPartial(Term_p t, Term_p s, Subst_p subst)
{
   UnificationResult res;
   if(problemType == PROBLEM_FO)
   {
      res = (UnificationResult) {SubstComputeMgu(t,s,subst) ? RightTerm : NoTerm, 0};
   }
   else
   {
      res = SubstComputeMguHO(t,s,subst);
   }

   return res;
}
/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


