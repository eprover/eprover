/*-----------------------------------------------------------------------

File:    cto_lpogen.c

Author:  Joachim Steinbach

Contents:
         

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes
         New

-----------------------------------------------------------------------*/

#include "cto_lpo.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/
static CompareResult extreg(OCB_p, Term_p, Term_p,
			    DerefType, DerefType, PStack_p);

static CompareResult fungtr(OCB_p, Term_p, Term_p,
			    DerefType, DerefType, PStack_p);

static CompareResult funeq(OCB_p, Term_p, Term_p,
			   DerefType, DerefType, PStack_p);

static CompareResult checkarg(OCB_p, Term_p, Term_p,
			      DerefType, DerefType, PStack_p);

/*-----------------------------------------------------------------------
//
// Function: extreg(ocb, s, t, ds, dt, ocbstack)
//
//           tries to extend a registry (i.e., an ocb)
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult extreg(OCB_p ocb, Term_p s, Term_p t,
			    DerefType deref_s, DerefType deref_t,
			    PStack_p ocbstack)
{
   CompareResult res, resfuns;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

                     /* The cases s=x v t=x are checked separately. */
   if (TermIsVar(s) || TermIsVar(t))
   {
      switch ((res = LPOCompareVars(s, t, deref_s, deref_t)))
      {
      case to_greater: case to_equal:
	    return res;
	    break;
      case to_lesser: case to_uncomparable:
	    return to_uncomparable;
	    break;
      default:
	    assert(false);
	    break;
      }
   }

   switch ((resfuns = OCBFunCompare(ocb, s->f_code, t->f_code)))
   {
   case to_greater:
	 res = fungtr(ocb, s, t, deref_s, deref_t, ocbstack);
	 break;
   case to_equal:
	 res = funeq(ocb, s, t, deref_s, deref_t, ocbstack);
	 break;
   case to_lesser:
	 switch (fungtr(ocb, t, s, deref_t, deref_s, ocbstack))
	 {
	 case to_uncomparable:
	       res = to_uncomparable;
	       break;
	 case to_greater:
	       res = to_lesser;
	       break;
	 case to_lesser:
	       res = to_greater;
	       break;
	 default:
                    /* Note that fungtr does not return `to_equal'. */
	       assert(false);
	       break;
	 }
	 break;
   default:         /* Leave the switch statement whenever the leading
                    // operators are incomparable.                  */
	 break;
   }

   if ((res == to_greater) || (res == to_equal))
   {
      return res;
   }

           /* Check the third condition of the lpo (see `(iii)' of the
           // definition at the top of file cto_lpo.c.              */
   if ((resfuns != to_greater) &&
       ((resfuns != to_equal) || (s->arity >= 2)) &&
       (checkarg(ocb, s, t, deref_s, deref_t, ocbstack)
	== to_greater))
   {
      return to_greater;
   }

   if (resfuns == to_uncomparable)
   {
      OCBPrecedenceAddTuple(ocb, s->f_code, t->f_code, to_greater);
      falls konsistent --> testen, ob jetzt groesser
      wenn ja, return to_greater
   }

   backtracking ueber ocb (zuruecknehmen von Relationen)

}

/*-----------------------------------------------------------------------
//
// Function: fungtr(ocb, s, t, ds, dt, ocbstack)
//
// Compares the term s with the arguments t_1,...,t_n of t and returns
//
//       to_greater        if s >LPO t_1, ..., s >LPO t_n
//       to_lesser         if there exists t_i with t_i >LPO s
//       to_uncomparable   otherwise
//
// when using the current registry ocb
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult fungtr(OCB_p ocb, Term_p s, Term_p t,
			    DerefType deref_s, DerefType deref_t,
			    PStack_p ocbstack)
{
   bool greater = true;
   int i;

   for (i=0; i<t->arity; i++)
   {
      switch (extreg(ocb, s, t->args[i], deref_s, deref_t, ocbstack))
      {
      case to_greater:
	    break;
      case to_lesser: case to_equal:
	    return to_lesser;
	    break;
      case to_uncomparable:
	    greater = false;
	    break;
      default:
	    assert(false);
	    break;
      }
   }

   if (greater)
   {
      return to_greater;
   }
   return to_uncomparable;
}


/*-----------------------------------------------------------------------
//
// Function: funeq(ocb, s, t, ds, dt, ocbstack)
//
// Compares the arguments s_1,...,s_m of the term s with the arguments
// t_1,...,t_n of the term t and returns
//
//        to_equal          if m=n & s_1 =LPO t_1, ..., s_m =LPO t_m
//        to_greater        if (s_1,...,s_m) >LPOlex (t_1,...,t_n)  &
//                             forall t_i: s >LPO t_i
//        to_lesser         if (t_1,...,t_n) >LPOlex (s_1,...,s_n)  &
//                             forall s_i: t >LPO s_i
//        to_uncomparable   otherwise
//
// when using the current registry ocb
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult funeq(OCB_p ocb, Term_p s, Term_p t,
			   DerefType deref_s, DerefType deref_t,
			   PStack_p ocbstack)
{
   CompareResult res = to_equal, res_help, comp_res;
   int i, j;
   
   for (i=0; i<MAX(s->arity,t->arity); i++)
   {
      if (t->arity <= i)
      {
	 return to_greater;
      }
      if (s->arity <= i)
      {
	 return to_lesser;
      }
      if ((res = extreg(ocb, s->args[i], t->args[i],
			deref_s, deref_t, ocbstack))
	  == to_uncomparable)
      {
	 return to_uncomparable;
      }
      if ((res == to_greater) || (res == to_lesser))
      {
	 res_help = res;
	 comp_res = (res == to_greater) ? to_lesser : to_greater;
	 
	 j = i+1;
	 while ((j<MAX(s->arity, t->arity)) && (res_help == res))
	 {
	    if (res == to_greater)
	    {
	       res_help = extreg(ocb, s, t->args[j],
				 deref_s, deref_t, ocbstack);
	    }
	    else
	    {
	       res_help = extreg(ocb, s->args[j], t,
				 deref_s, deref_t, ocbstack);
	    }
	    if (res_help == comp_res)
	    {
	       return comp_res;
	    }
	    j++;
	 }
	 if (res_help == res)
	 {
	    return res;
	 }
	 return to_uncomparable;
      }
   }
   return to_equal;
}

/*-----------------------------------------------------------------------
//
// Function: checkarg(ocb, s, t, ds, dt, ocbstack)
//
// Checks the third condition of the LPO, i.e. returns
//
//        to_greater        if there is an argument s_i of s
//                             with s_i >=LPO t
//        to_uncomparable   otherwise
//
// when using the current registry ocb
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult checkarg(OCB_p ocb, Term_p s, Term_p t,
			      DerefType deref_s, DerefType deref_t,
			      PStack_p ocbstack)
{
   CompareResult res;
   int i;
   
   for (i=0; i<s->arity; i++)
   {
      res = extreg(ocb, s->args[i], t, deref_s, deref_t, ocbstack);
      if ((res == to_greater) || (res == to_equal))
      {
	 return to_greater;
      }
   }
   return to_uncomparable;
}
