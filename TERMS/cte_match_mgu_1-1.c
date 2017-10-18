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
         assert(matcher->sort != STNoSort);
         assert(to_match->sort != STNoSort);
         if(matcher->sort != to_match->sort)
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

bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst)
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
            assert(t1->sort != STNoSort);
            assert(t2->sort != STNoSort);
            /* Sort check and occur check - remember, variables are elementary and shared! */
            if((t1->sort != t2->sort) || occur_check(t2, t1))
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
            assert(t1->sort != STNoSort);
            assert(t2->sort != STNoSort);
            assert(t1->sort == t2->sort);
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


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


