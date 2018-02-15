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

int PartiallyMatchVar(Term_p var_matcher, Term_p to_match, Sig_p sig, bool perform_occur_check)
{
   assert(TermIsVar(var_matcher) && !var_matcher->binding);
   assert(!TermIsAppliedVar(to_match) || to_match->f_code == sig->app_var_code);
   assert(ProblemIsHO == PROBLEM_IS_HO || !TypeIsArrow(var_matcher->type));
   

   int args_to_eat = NOT_MATCHED;
   Type_p term_head_type = GetHeadType(sig, to_match);
   Type_p matcher_type   = var_matcher->type;

   if (matcher_type == to_match->type)
   {
      args_to_eat = ARG_NUM(to_match);
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

      args_to_eat = start_idx;
      // if they have the same nr of args and args match -> they're the same
      // -> nice place to check the type sharing invariant
      assert(args_to_eat != 0 || matcher_type == term_head_type);
   }

   /* The case where we could eat up arguments, but they are not there. */
   if (args_to_eat > ARG_NUM(to_match))
   {
      return NOT_MATCHED;
   }

   if (perform_occur_check)
   {
      for(int i=0; i<args_to_eat + TermIsAppliedVar(to_match) ? 1 : 0; i++)
      {
         if (occur_check(to_match->args[i], var_matcher))
         {
            return NOT_MATCHED;
         }
      }
   }


   // the number of arguments eaten
   return args_to_eat;
}

#define FAIL_AND_BREAK(res, val) { (res) = (val); break; }

int SubstComputeMatchHO(Term_p matcher, Term_p to_match, Subst_p subst, Sig_p sig)
{
   assert(ProblemIsHO == PROBLEM_IS_HO);
   long matcher_weight  = TermStandardWeight(matcher);
   long to_match_weight = TermStandardWeight(to_match);

   assert(TermStandardWeight(matcher)  == TermWeight(matcher, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == TermWeight(to_match, DEFAULT_VWEIGHT, DEFAULT_FWEIGHT));

   TermPrint(stderr, matcher, sig, DEREF_NEVER);
   fprintf(stderr, " =m?> ");
   TermPrint(stderr, to_match, sig, DEREF_NEVER);
   fprintf(stderr, ".\n");

   int res = MATCH_INIT;
   if(matcher_weight > to_match_weight)
   {
      return NOT_MATCHED;
   }

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

      if (TermIsTopLevelVar(matcher))
      {
         Term_p var = TermIsAppliedVar(matcher) ? matcher->args[0] : matcher;

         if (var->binding)
         {
            // by now there is no prefix matching, since we already bound sthg
            assert(res != MATCH_INIT); 
            if (TermIsPrefix(var->binding, to_match))
            {
               start_idx = ARG_NUM(var->binding);
               assert(ARG_NUM(matcher) == 
                        to_match->arity - start_idx - TermIsAppliedVar(to_match) ? 1 :0);
               
               matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
               if(matcher_weight > to_match_weight)
               {
                  FAIL_AND_BREAK(res, NOT_MATCHED);
               }
            }
            else
            {
               FAIL_AND_BREAK(res, NOT_MATCHED);
            }
         }
         else
         {
            int args_eaten = PartiallyMatchVar(var, to_match, sig, false);
            if (args_eaten == NOT_MATCHED)
            {
               FAIL_AND_BREAK(res, NOT_MATCHED);
            }
            else
            {
               SubstBindAppVar(subst, var, to_match, args_eaten);                  
               start_idx = args_eaten;

               matcher_weight += TermStandardWeight(var->binding) - DEFAULT_VWEIGHT;
               if(matcher_weight > to_match_weight)
               {
                  FAIL_AND_BREAK(res, NOT_MATCHED);
               }

               if (args_eaten + ARG_NUM(matcher) != ARG_NUM(to_match))
               {
                  assert(res == MATCH_INIT);
                  res = ARG_NUM(to_match) - args_eaten + ARG_NUM(matcher);
               }
            }
         }
      }
      else
      {
         if (matcher->f_code != to_match->f_code 
               || matcher->arity > to_match ->arity)
         {
            FAIL_AND_BREAK(res, NOT_MATCHED);
         }
         else
         {
            start_idx = 0;

            if (ARG_NUM(matcher) != ARG_NUM(to_match))
            {
               assert(res == MATCH_INIT);
               res = ARG_NUM(to_match) - ARG_NUM(matcher);
            }
         }
      }

      const int offset = start_idx + (TermIsAppliedVar(to_match) ? 1 : 0)
                                   - (TermIsAppliedVar(matcher) ? 1 : 0);
      fprintf(stderr, "offset = %d, start_idx = %d, |matcher| = %d, |to_match| = %d\n", 
                      offset, start_idx, matcher->arity, to_match->arity);
      assert(matcher->arity + offset <= to_match->arity);
      
      PLocalStackEnsureSpace(jobs, 2*(matcher->arity));
      for(int i=TermIsAppliedVar(matcher) ? 1 : 0; i<matcher->arity; i++)
      {
         PLocalStackPush(jobs, matcher->args[i]);
         PLocalStackPush(jobs, to_match->args[i+offset]);
      }
   }

   if(res == NOT_MATCHED)
   {
      SubstBacktrackToPos(subst, backtrack);
   }

   PLocalStackFree(jobs);
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
   assert(ProblemIsHO == PROBLEM_IS_HO);
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

static __inline__ bool reorientation_needed(Term_p t1, Term_p t2)
{
   if (TermIsTopLevelVar(t2))
   {
      return !TermIsTopLevelVar(t1) ||
               TypeGetSymbolArity(GetHeadType(NULL, t2)) <
               TypeGetSymbolArity(GetHeadType(NULL, t1)) ||
               (TypeGetSymbolArity(GetHeadType(NULL, t2)) ==
               TypeGetSymbolArity(GetHeadType(NULL, t1)) && t2->arity < t1->arity);
   }
   else
   {
      return !TermIsTopLevelVar(t1) && t1->f_code == t2->f_code &&
               t2->arity < t1->arity;
   }
}



UnificationResult SubstComputeMguHO(Term_p t1, Term_p t2, Subst_p subst, Sig_p sig)
{   
   #ifdef MEASURE_UNIFICATION
      UnifAttempts++;
   #endif
   PERF_CTR_ENTRY(MguTimer);
   assert(ProblemIsHO == PROBLEM_IS_HO);
   
   PStackPointer backtrack = PStackGetSP(subst);  //For backtracking 

   PQueue_p jobs = PQueueAlloc();
   bool swapped = reorientation_needed(t1, t2);
   UnificationResult res = UNIF_INIT;

   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);

   fprintf(stderr, "beginning: \n");
   TermPrint(stderr, t1, sig, DEREF_NEVER);
   fprintf(stderr, " (head type ");
   TypePrintTSTP(stderr, sig->type_bank, GetHeadType(sig, t1));
   fprintf(stderr, ") =?= ");
   TermPrint(stderr, t2, sig, DEREF_NEVER);
   fprintf(stderr, " (head type ");
   TypePrintTSTP(stderr, sig->type_bank, GetHeadType(sig, t2));
   fprintf(stderr, ").\n");

   while(!PQueueEmpty(jobs))
   {
      t2 =  TermDerefAlways(PQueueGetLastP(jobs));
      t1 =  TermDerefAlways(PQueueGetLastP(jobs));

      fprintf(stderr, "inside: \n");
      TermPrint(stderr, t1, sig, DEREF_NEVER);
      fprintf(stderr, " (head type ");
      TypePrintTSTP(stderr, sig->type_bank, GetHeadType(sig, t1));
      fprintf(stderr, ") =?= ");
      TermPrint(stderr, t2, sig, DEREF_NEVER);
      fprintf(stderr, " (head type ");
      TypePrintTSTP(stderr, sig->type_bank, GetHeadType(sig, t2));
      fprintf(stderr, ").\n");

      

      int start_idx;

      if (reorientation_needed(t1, t2))
      {
         SWAP(t1, t2);
         fprintf(stderr, "swapped.\n");
      }

      if (TermIsTopLevelVar(t1))
      {
         Term_p var = TermIsAppliedVar(t1) ? t1->args[0] : t1;
         
         int args_eaten = PartiallyMatchVar(var, t2, sig, true);
         if (args_eaten == NOT_MATCHED)
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }
         
         SubstBindAppVar(subst, var, t2, args_eaten);
         
         if (var->binding == var)
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

         if (t1->f_code != t2->f_code)
         {
            FAIL_AND_BREAK(res, UNIF_FAILED);
         }

         start_idx = 0;         
      }

      if (t1->arity > t2->arity)
      {
         // making sure that the argument with less arguments is on the left
         // previously we made sure that the variable is on the left.
         SWAP(t1, t2);
         swapped = !swapped;
      }

      const int offset = start_idx + (TermIsAppliedVar(t2) ? 1 : 0)
                                   - (TermIsAppliedVar(t1) ? 1 : 0);
      assert(t1->arity + offset <= t2->arity);
      assert(ARG_NUM(t2) == ARG_NUM(t1) + start_idx || UnifIsInit(res));

      fprintf(stderr, "|t1| = %d, |t2| = %d, offset = %d, start_idx = %d.\n",
                      t1->arity, t2->arity, offset, start_idx);

      if (UnifIsInit(res))
      {
         res = (UnificationResult){swapped ? RightTerm : LeftTerm, 
                                // args in t2 - eaten args - args in t1
                                ARG_NUM(t2) - start_idx - ARG_NUM(t1)};   
      }
     

      for(int i=TermIsAppliedVar(t1) ? 1 : 0; i<t1->arity; i++)
      {
         if(TermIsTopLevelVar(t1->args[i]) || TermIsTopLevelVar(t2->args[i+offset]))
         {
            PQueueBuryP(jobs, t2->args[i+offset]);
            PQueueBuryP(jobs, t1->args[i]);
         }
         else
         {
            PQueueStoreP(jobs, t1->args[i]);
            PQueueStoreP(jobs, t2->args[i+offset]);
         }  
      }
   }

   if (UnifFailed(res))
   {
      SubstBacktrackToPos(subst,backtrack);
      fprintf(stderr, "fail.\n");  
   }
   else
   {
      #ifdef MEASURE_UNIFICATION
         UnifSuccesses++;
      #endif
      fprintf(stderr, "substitution = ");
      SubstPrint(stderr, subst, sig, DEREF_ALWAYS);
      fprintf(stderr, ".\n");
      fprintf(stderr, "side = %s, remaining args = %d\n",
              res.term_side == LeftTerm ? "left" : "right", res.term_remaining);
   }

   PQueueFree(jobs);

   PERF_CTR_EXIT(MguTimer);
   return res;
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
   bool res;
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      res = SubstComputeMatch(t, s, subst);
   }
   else
   {
      // no arguments of s remaining after the match
      PStackPointer backtrack = PStackGetSP(subst);

      int res_i =  SubstComputeMatchHO(t, s, subst, sig);

      if (res_i != 0)
      {
         SubstBacktrackToPos(subst, backtrack);
      }
      res = res_i == 0;
   }

   return res;
}

__inline__ bool SubstMguComplete(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   bool res;
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      res = SubstComputeMgu(t, s, subst);
   }
   else
   {
      // no arguments of s remaining after the match
      PStackPointer backtrack = PStackGetSP(subst);
      UnificationResult u_res =  SubstComputeMguHO(t, s, subst, sig);

      
      if (UnifFailed(u_res) || u_res.term_remaining != 0)
      {
         SubstBacktrackToPos(subst, backtrack);
      }
      
      res = !UnifFailed(u_res) && u_res.term_remaining == 0;
   }  

   return res;
}

__inline__ int SubstMatchPossiblyPartial(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   int res;
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      res = SubstComputeMatch(t, s, subst) ? 0 : NOT_MATCHED;
   }
   else
   {
      res = SubstComputeMatchHO(t, s, subst, sig);
   }

   return res;
}
#endif

UnificationResult SubstMguPossiblyPartial(Term_p t, Term_p s, Subst_p subst, Sig_p sig)
{
   UnificationResult res;
   if (ProblemIsHO == PROBLEM_NOT_HO)
   {
      res = (UnificationResult) {SubstComputeMgu(t,s,subst) ? RightTerm : NoTerm, 0};
   }
   else
   {
      res = SubstComputeMguHO(t,s,subst,sig);
   }

   return res;
}
/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


