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



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#ifdef NEVER_DEFINED /* Inefficient in the linear version */

/*-----------------------------------------------------------------------
//
// Function: check_unify_job()
//
//   Return true is the pair (t1,t2) is already on the stacks.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool check_unify_job(PStack_p s1, PStack_p s2, Term_p t1,
			    Term_p t2)
{
   PStackPointer i, limit;

   assert(t1);
   assert(t2);
   assert(s1);
   assert(s2);

   limit = PStackGetSP(s1);
   assert(limit == PStackGetSP(s2));

   for(i=limit-1; i>=0; i--)
   {
      if((t1==PStackElementP(s1,i))&&(t2==PStackElementP(s2,i)))
      {
	 return true;
      }
   }
   return false;
}

#endif


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

static bool occur_check(Term_p super, Term_p var)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool occ = false;
   DerefType deref = DEREF_ALWAYS;

   PStackPushP(stack, super);
   
   while(!PStackEmpty(stack))
   {
      super = PStackPopP(stack);

      super = TermDeref(super, &deref);

      if(super == var)
      {
         occ = true;
         break;
      }
      else
      {
         for(i=0; i<super->arity; i++)
         {
            PStackPushP(stack, super->args[i]);
         }
      }
   }
   PStackFree(stack);

   return occ;
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
//   For shared terms, calling the function with TBTermEqual() as the
//   last argument (the function used to test terms for equality)
//   results in more efficient execution (otherwise call the function
//   with TermBasicStructEqual()). 
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

bool SubstComputeMatch(Term_p matcher, Term_p to_match, Subst_p subst,
		       TermEqualTestFun EqualTest)
{
   long matcher_weight = TermStandardWeight(matcher),
      to_match_weight = TermStandardWeight(to_match);
   bool res = true;

   assert(TermStandardWeight(matcher)  == 
	  TermWeight(matcher,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
   assert(TermStandardWeight(to_match) == 
	  TermWeight(to_match,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));

   if(matcher_weight > to_match_weight)
   {
      return false;
   }	
   if(TermCellQueryProp(to_match, TPPredPos) && TermIsVar(matcher))
   {
      return false;
   }
   /* New block to get fresh local variables */
   {
      PStack_p jobs = PStackAlloc();
      PStackPointer backtrack = PStackGetSP(subst); /* For backtracking */
      int i;
      
      PStackPushP(jobs, matcher);
      PStackPushP(jobs, to_match);
      
      while(!PStackEmpty(jobs))
      {
	 to_match =  PStackPopP(jobs);
	 matcher  =  PStackPopP(jobs);
	 
	 if(TermIsVar(matcher))
	 {
	    if(matcher->binding)
	    {
	       if(!EqualTest(matcher->binding,to_match))
	       {
		  res = false;
		  break;
	       }
	    }
	    else 
	    {
	       SubstAddBinding(subst, matcher, to_match);
	       
	    }
	    matcher_weight +=
	       (TermStandardWeight(to_match)-DEFAULT_VWEIGHT);
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
	       for(i=matcher->arity-1; i>=0; i--)
	       {
		  PStackPushP(jobs, matcher->args[i]);
		  PStackPushP(jobs, to_match->args[i]);
	       }
	    }
	 }
      }      
      PStackFree(jobs);
      if(!res)
      {
	 SubstBacktrackToPos(subst,backtrack);
      }
      return res;
   }
}


/*-----------------------------------------------------------------------
//
// Function: SubstComputeMgu()
//
//   Compute an mgu between two terms. Currently without any special
//   optimization, except for checking for double entries in the
//   to-solve stack (deleted, ineficient). Returns true and modifies
//   subst if sucessful, false otherwise (as for match, see
//   above). Terms have to be variable disjoint, otherwise behaviour
//   is unpredictable!
//
//   Solution with stacks is more efficient than unsorted queues,
//   sorted queues are significantly better again!
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

bool SubstComputeMgu(Term_p t1, Term_p t2, Subst_p subst)
{
   PQueue_p jobs;
   PStackPointer backtrack; /* For backtracking */
   int i;
   DerefType deref = DEREF_ALWAYS;

   if((TermCellQueryProp(t1, TPPredPos) && TermIsVar(t2))||
      (TermCellQueryProp(t2, TPPredPos) && TermIsVar(t1)))
   {
      return false;
   }
   jobs = PQueueAlloc();
   backtrack = PStackGetSP(subst);
   
   PQueueStoreP(jobs, t1);
   PQueueStoreP(jobs, t2);
   
   while(!PQueueEmpty(jobs))
   {
      t2 =  TermDeref(PQueueGetLastP(jobs), &deref);
      t1 =  TermDeref(PQueueGetLastP(jobs), &deref);	    
      
      if(TermIsVar(t2))
      {
         SWAP(Term_p, t1, t2);
      }

      if(TermIsVar(t1))
      {
	 if(!TBTermEqual(t1,t2))
	 {
	    /* Occur-Check - remember, variables are elementary and
	       shared! */
	    if(occur_check(t2, t1))
	    {
	       SubstBacktrackToPos(subst,backtrack);
	       PQueueFree(jobs);
	       return false;
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
	    SubstBacktrackToPos(subst,backtrack);
	    PQueueFree(jobs);
	    return false;
	 }
	 else
	 {
	    for(i=t1->arity-1; i>=0; i--)
	    {	
	       /* Delay variable bindings */
	       if(TermIsVar(t1->args[i])||TermIsVar(t2->args[i]))
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
   return true;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


