/*-----------------------------------------------------------------------

File:    cto_lpo.c

Author:  Stephan Schulz (original implementation and definitions by
         Joachim Steinbach)

Contents:
         Implementation of a lexicographic path ordering (LPO) on CLIB
	 terms. It is based on the following refined definition:

	 s = f(s_1,...,s_m) >LPO g(t_1,...,t_n) = t  iff

	   f > g  &  forall j in [1,n]: s >LPO t_j   or             (i)
	   f ~ g  &  (s_1,...,s_m) >LPOlex (t_1,...,t_n)  &        (ii)
	             forall j in [1,n]: s >LPO t_j   or
	   not(f > g)  &  (not(f ~ g) v ar(f) >= 2)  &            (iii)
	             there exists i in [1,m]: s_i >=LPO t

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes
         <1> Unknown
             New
	 <2> Mon May 18 10:43:15 MET DST 1998
	     Changed
	 <3> Tue Jun 16 13:05:31 MET DST 1998
	     Changed
	 <4> Tue Sep 15 08:53:35 MET DST 1998
	     Changed
	 <5> Sat Jan  8 00:22:20 MET 2000
             StS: Rewrote the complete code base to make it more
             maintainable and to add caching.

-----------------------------------------------------------------------*/

#include "cto_lpo.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long LPORecursionDepthLimit = 1000;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static bool lpo_term_dominates_args(OCB_p ocb, Term_p s, Term_p t,
				     DerefType deref_s, DerefType
				     deref_t);

static bool lpo_subterm_dominates_term(OCB_p ocb, Term_p s, Term_p t,
				       DerefType deref_s, DerefType
				       deref_t);

static CompareResult lpo_lex_greater(OCB_p ocb, Term_p s, Term_p t,
				     DerefType deref_s, DerefType
				     deref_t);

static CompareResult lpo_greater(OCB_p ocb, Term_p s, Term_p t,
				 DerefType deref_s, DerefType
				 deref_t);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: lpo_term_dominates_args()
//
//   Return true if s >LPO t_i for all subterms t_i of t, false
//   otherwise. 
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo_term_dominates_args(OCB_p ocb, Term_p s, Term_p t,
				    DerefType deref_s, DerefType
				    deref_t)
{
   int i;

   for(i=0; i<t->arity;i++)
   {
      if(!LPOGreater(ocb, s, t->args[i], deref_s, deref_t))
      {
	 return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: lpo_subterm_dominates_term()
//
//   Return true if s_i >=LPO t for a direct subterm of s.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo_subterm_dominates_term(OCB_p ocb, Term_p s, Term_p t,
				       DerefType deref_s, DerefType
				       deref_t)
{
   int           i;
   CompareResult res;
   
   for(i=0; i<s->arity; i++)
   {
      res = lpo_greater(ocb, s->args[i], t, deref_s, deref_t);
      if((res==to_greater)||(res==to_equal))
      {
	 return true;
      }
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: lpo_lex_greater()
//
//   Compare the arguments of s and t. Return 
//
//   to_greater      if s1...sn >_LPO_lex  t1...tm
//   to_equal        if s1...sn ~LPO_lex   t1...tm
//   to_uncomparable if s1...sn =!=LPO_lex t1...tm
//   to_notgteq      otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult lpo_lex_greater(OCB_p ocb, Term_p s, Term_p t,
				     DerefType deref_s, DerefType
				     deref_t)
{
   int           i, lim;
   CompareResult res;

   lim = MIN(s->arity, t->arity);
   res = to_equal;

   for(i=0; i<lim;i++)
   {
      res = lpo_greater(ocb, s->args[i], t->args[i], deref_s,
			deref_t);
      if(res!=to_equal)
      {
	 break;
      }
   }
   if(res == to_equal)
   {
      if(s->arity > t->arity)
      {
	 res = to_greater;
      }
      else if(s->arity < t->arity)
      {
	 res = to_notgteq;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: lpo_greater()
//
//   Check if s >lpo t. Return
//   
//   to_equal        if s = t
//   to_greater      if s >lpo t
//   to_uncomparable if s and t are definitly uncomparable
//   to_nogteq       otherwise
//
// Global Variables: LPORecursionDepthLimit
//
// Side Effects    : Manipulates static variable recursion_depth
//
/----------------------------------------------------------------------*/

static CompareResult lpo_greater(OCB_p ocb, Term_p s, Term_p t,
				 DerefType deref_s, DerefType
				 deref_t)
{
   CompareResult res = to_notgteq, res2;
   static long   recursion_depth = 0;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if(recursion_depth > LPORecursionDepthLimit)
   {
      return to_uncomparable;
   }
   recursion_depth++;
   
   if(TermIsVar(s))
   {
      if(TBTermEqual(s,t))
      {
	 res = to_equal;
      }
      else if(TermIsVar(t))
      {
	 res = to_uncomparable;
      }
   }
   else if(TermIsVar(t))
   {
      if(TermIsSubterm(s, t, deref_s, TBTermEqual))
      {
	 res = to_greater;
      }
      else
      {
	 res = to_uncomparable;
      }
   }
   else
   {
      switch(OCBFunCompare(ocb, s->f_code, t->f_code))
      {
      case to_greater:	    
	    if(lpo_term_dominates_args(ocb, s, t, deref_s, deref_t))
	    {
	       res = to_greater;
	    }
	    break;
      case to_equal:
	    res2 = lpo_lex_greater(ocb, s, t, deref_s, deref_t);
	    switch(res2)
	    {
	    case to_greater:
		  if(lpo_term_dominates_args(ocb, s, t, deref_s,
					     deref_t))
		  {
		     res = to_greater;
		  }
		  break;
	    case to_equal:
		  res = to_equal;
		  break;
	    default:
		  break;		  
	    }
	    if((res==to_notgteq)&&(s->arity>=2)&&
	       lpo_subterm_dominates_term(ocb, s, t, deref_s, deref_t))
	    {
	       res = to_greater;
	    }
	    break;
      case to_lesser:
      case to_uncomparable:
	    if(lpo_subterm_dominates_term(ocb, s, t, deref_s,
					  deref_t))
	    {
	       res = to_greater;
	    }
	    break;
      default:
	    assert(false&&
		   "Unexpected result of function symbol comparison");
	    break;
      }
   }
   recursion_depth--;
   return res;
}
	 

/*---------------------------------------------------------------------*/
/*                      Exported Functions                             */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: LPOGreater(ocb, s, t, ds, dt)
//
//   Checks whether the term s is greater than the term t in the
//   Lexicographic Path Ordering (LPO), i.e. returns
//
//                       true      if s >LPO t,
//      		 false     otherwise.
//
//   For a description of the LPO see the header of this file.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

bool LPOGreater(OCB_p ocb, Term_p s, Term_p t,
		DerefType deref_s, DerefType deref_t)
{
   return lpo_greater(ocb, s, t, deref_s, deref_t) == to_greater;
}


/*-----------------------------------------------------------------------
//
// Function: LPOCompare(ocb, s, t, ds, dt)
//
//   Compare two terms s,t in the Lexicographic Path Ordering,
//   return the result
//                          to_greater         if s >LPO t
//                          to_equal           if s =LPO t
//                          to_lesser          if t >LPO s
//                          to_uncomparable    otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

CompareResult LPOCompare(OCB_p ocb, Term_p s, Term_p t,
			 DerefType deref_s, DerefType deref_t)
{
   CompareResult res;

   res = lpo_greater(ocb, s, t, deref_s, deref_t);
   switch(res)
   {
   case to_greater:
   case to_lesser: /* From caching only */
   case to_equal:
   case to_uncomparable:
	 return res;
	 break;
   case to_notleeq:
   case to_unknown:
	 assert(false);
	 break;
   case to_notgteq:
	 break;
   }
   res = lpo_greater(ocb, t, s, deref_t, deref_s);
   switch(res)
   {
   case to_greater:
	 return to_lesser;
	 break;
   case to_uncomparable:
   case to_notgteq:
	 break;
   case to_equal:
   case to_lesser:
   case to_notleeq:
   case to_unknown:
	 assert(false);
	 break;
   }
   return to_uncomparable;	 
}
