/*-----------------------------------------------------------------------

File:    cto_lpo.c

Author:  Joachim Steinbach

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
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes
         New
    <2> Mon May 18 10:43:15 MET DST 1998
    Changed
    <3> Tue Jun 16 13:05:31 MET DST 1998
    Changed
    <4> Tue Sep 15 08:53:35 MET DST 1998
    Changed

-----------------------------------------------------------------------*/

#include "cto_lpo_debug.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/
static CompareResult lpofungtr(OCB_p, Term_p, Term_p,
                DerefType, DerefType);

static CompareResult lpofuneq(OCB_p, Term_p, Term_p,
               DerefType, DerefType);

static CompareResult lpocheckarg(OCB_p, Term_p, Term_p,
             DerefType, DerefType);

static CompareResult lpogtr(OCB_p, Term_p, Term_p,
             DerefType, DerefType);

static CompareResult lpogtrcompvars(OCB_p, Term_p, Term_p,
                DerefType, DerefType);

static CompareResult lpogtrfuneq(OCB_p, Term_p, Term_p,
             DerefType, DerefType);

static CompareResult lpogtrcheckarg(OCB_p, Term_p, Term_p,
                DerefType, DerefType);


/*-----------------------------------------------------------------------
//
// Function: lpofungtr(ocb, s, t, ds, dt)
//
// Compares the term s with the arguments t_1,...,t_n of t and returns
//
//       to_greater        if s >LPO t_1, ..., s >LPO t_n
//       to_lesser         if there exists t_i with t_i >LPO s
//       to_uncomparable   otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult lpofungtr(OCB_p ocb, Term_p s, Term_p t,
                DerefType deref_s, DerefType deref_t)
{
   bool greater = true;
   int i;

   for (i=0; i<t->arity; i++)
   {
      switch (D_LPOCompare(ocb, s, t->args[i], deref_s, deref_t))
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
// Function: lpofuneq(ocb, s, t, ds, dt)
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
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult lpofuneq(OCB_p ocb, Term_p s, Term_p t,
               DerefType deref_s, DerefType deref_t)
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
      if ((res = D_LPOCompare(ocb, s->args[i], t->args[i],
             deref_s, deref_t)) == to_uncomparable)
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
          res_help = D_LPOCompare(ocb, s, t->args[j],
                 deref_s, deref_t);
       }
       else
       {
          res_help = D_LPOCompare(ocb, s->args[j], t,
                 deref_s, deref_t);
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
// Function: lpocheckarg(ocb, s, t, ds, dt)
//
// Checks the third condition of the LPO, i.e. returns
//
//        to_greater        if there is an argument s_i of s
//                             with s_i >=LPO t
//        to_uncomparable   otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult lpocheckarg(OCB_p ocb, Term_p s, Term_p t,
             DerefType deref_s, DerefType deref_t)
{
   CompareResult res;
   int i;

   for (i=0; i<s->arity; i++)
   {
      res = D_LPOCompare(ocb, s->args[i], t, deref_s, deref_t);
      if ((res == to_greater) || (res == to_equal))
      {
    return to_greater;
      }
   }
   return to_uncomparable;
}

/*-----------------------------------------------------------------------
//
// Function: lpogtr(ocb, s, t, ds, dt)
//
//   Returns
//              to_greater         if s >LPO t,
//              to_equal           if s =LPO t,
//              to_uncomparable    otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/
static CompareResult lpogtr(OCB_p ocb, Term_p s, Term_p t,
             DerefType deref_s, DerefType deref_t)
{
   CompareResult res, res_funs;
   int i;

   s = TermDeref(s, &deref_s);  t = TermDeref(t, &deref_t);

   /* The cases s=x v t=x are checked separately. */
   if(TermIsFreeVar(s) || TermIsFreeVar(t))
   {
      res = lpogtrcompvars(ocb, s, t, deref_s, deref_t);
      return res;
   }

   /* Depending on the comparison of the leading operators wrt. the
      precedence, different conditions must be tested. */

   switch((res_funs = OCBFunCompare(ocb, s->f_code, t->f_code)))
   {
   case to_greater:
    res = to_greater;
    for(i=0; i<t->arity; i++)
    {
       if((res = lpogtr(ocb, s, t->args[i], deref_s, deref_t)) !=
          to_greater)
       {
          break;
       }
    }
    if(res == to_greater)
    {
       return res;
    }
    break;
   case to_lesser:
    for(i=0; i<s->arity; i++)
    {
       res = lpogtr(ocb, s->args[i], t, deref_s, deref_t);
       if((res == to_greater) || (res == to_equal))
       {
          return to_greater;
       }
    }
    return to_uncomparable;
    break;
   case to_equal:
    if ((res = lpogtrfuneq(ocb, s, t, deref_s, deref_t)) !=
        to_uncomparable)
    {
       return res;
    }
    break;
   default:            /* Leave the switch statement whenever the leading
                          operators are incomparable.                  */
    break;
   }

   if ((res_funs != to_greater) &&
       ((res_funs != to_equal) || (s->arity >= 2)))
   {
      res = lpogtrcheckarg(ocb, s, t, deref_s, deref_t);
      return res;
   }
   return to_uncomparable;
}


/*-----------------------------------------------------------------------
//
// Function: lpogtrcompvars(ocb, s, t, ds, dt)
//
// Compares two terms s and t wrt. LPO if either s or t is a variable
// and returns
//               to_greater        if t is a subterm of s
//               to_equal          if s == t
//               to_uncomparable   otherwise
//
// If s is a variable, then varps is true.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult lpogtrcompvars(OCB_p ocb, Term_p s, Term_p t,
                DerefType deref_s,
                DerefType deref_t)
{
   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if (TermIsFreeVar(s))
   {
      if(s == t)
      {
    return to_equal;
      }
      else
      {
    return to_uncomparable;
      }
   }
   else
   {
      assert(TermIsFreeVar(t));
      if(TermIsSubterm(s, t, deref_s))
      {
    return to_greater;
      }
   }
   return to_uncomparable;
}


/*-----------------------------------------------------------------------
//
// Function: lpogtrfuneq(ocb, s, t, ds, dt)
//
// Compares the arguments s_1,...,s_m of the term s with the arguments
// t_1,...,t_n of the term t and returns
//
//        to_equal          if m=n & s_1 =LPO t_1, ..., s_m =LPO t_m
//        to_greater        if (s_1,...,s_m) >LPOlex (t_1,...,t_n)  &
//                             forall t_i: s >LPO t_i
//        to_uncomparable   otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult lpogtrfuneq(OCB_p ocb, Term_p s, Term_p t,
                  DerefType deref_s, DerefType deref_t)
{
   CompareResult res = to_equal;
   int i, j;

   for (i=0; i<MAX(s->arity, t->arity); i++)
   {
      if (t->arity <= i)
      {
    return to_greater;
      }
      if (s->arity <= i)
      {
    return to_uncomparable;
      }

      res = lpogtr(ocb, s->args[i], t->args[i], deref_s, deref_t);
      if (res == to_uncomparable)
      {
    break;
      }
      if (res == to_greater)
      {
    j = i+1;
    while ((j<MAX(s->arity, t->arity)) && (res == to_greater))
    {
       res = lpogtr(ocb, s, t->args[j], deref_s, deref_t);
       j++;
    }
    if (res == to_greater)
    {
       return to_greater;
    }
    return to_uncomparable;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: lpogtrcheckarg(ocb, s, t, ds, dt)
//
// Checks the third condition of the LPO, i.e. returns
v//
//        to_greater        if there is an argument s_i of s
//                             with s_i >=LPO t
//        to_uncomparable   otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static CompareResult lpogtrcheckarg(OCB_p ocb, Term_p s, Term_p t,
                DerefType deref_s,
                DerefType deref_t)
{
   CompareResult res;
   int i;

   for (i=0; i<s->arity; i++)
   {
      res = lpogtr(ocb, s->args[i], t, deref_s, deref_t);
      if ((res == to_greater) || (res == to_equal))
      {
    return to_greater;
      }
   }
   return to_uncomparable;
}


/*---------------------------------------------------------------------*/
/*                      Exported Functions                             */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: D_LPOCompare(ocb, s, t, ds, dt)
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

CompareResult D_LPOCompare(OCB_p ocb, Term_p s, Term_p t,
          DerefType deref_s, DerefType deref_t)
{
   assert(problemType != PROBLEM_HO); // no need to change derefs
   CompareResult res, res_funs;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

                     /* The cases s=x v t=x are checked separately. */
   if (TermIsFreeVar(s) || TermIsFreeVar(t))
   {
      res = D_LPOCompareVars(s, t, deref_s, deref_t);
      return res;
   }

   /* Depending on the comparison of the leading operators wrt. the
      precedence, different conditions must be tested.           */
   switch ((res_funs = OCBFunCompare(ocb, s->f_code, t->f_code)))
   {
   case to_greater:
    if ((res = lpofungtr(ocb, s, t, deref_s, deref_t)) !=
        to_uncomparable)
    {
       assert(res != to_equal);
       return res;
    }
    break;
   case to_equal:
    if ((res = lpofuneq(ocb, s, t, deref_s, deref_t)) !=
        to_uncomparable)
    {
       return res;
    }
    break;
   case to_lesser:
    if ((res = lpofungtr(ocb, t, s, deref_t, deref_s)) !=
        to_uncomparable)
    {
       /* Note that lpofungtr does not return `to_equal'. */
       res = (res == to_greater) ? to_lesser : to_greater;
       return res;
    }
    break;
   default:         /* Leave the switch statement whenever the leading
                       operators are incomparable.                  */
    break;
   }
           /* Check the third condition of the lpo (see `(iii)' of the
              definition at the top of this file.                   */
   if ((res_funs != to_greater) &&
       ((res_funs != to_equal) || (s->arity >= 2)) &&
       (lpocheckarg(ocb, s, t, deref_s, deref_t) == to_greater))
   {
      return to_greater;
   }
   if ((res_funs != to_lesser) &&
       ((res_funs != to_equal) || (t->arity >= 2)) &&
       (lpocheckarg(ocb, t, s, deref_t, deref_s) == to_greater))
   {
      return to_lesser;
   }
   return to_uncomparable;
}

/*-----------------------------------------------------------------------
//
// Function: D_LPOGreater(ocb, s, t, ds, dt)
//
//   Checks whether the term s is greater than the term t in the
//   Lexicographic Path Ordering (LPO), i.e. returns
//
//                       true      if s >LPO t,
//                       false     otherwise.
//
//   For a description of the LPO see the header of this file.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

bool D_LPOGreater(OCB_p ocb, Term_p s, Term_p t,
      DerefType deref_s, DerefType deref_t)
{
   assert(problemType != PROBLEM_HO); // no need to change derefs
   if (lpogtr(ocb, s, t, deref_s, deref_t) == to_greater)
   {
      return true;
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: D_LPOCompareVars(s, t, ds, dt)
//
// Compares two terms s and t wrt. LPO if either s or t is a variable
// and returns
//               to_greater        if t is a subterm of s
//               to_equal          if s == t
//               to_lesser         if s is a subterm of t
//               to_uncomparable   otherwise
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

CompareResult D_LPOCompareVars(Term_p s, Term_p t,
              DerefType deref_s,
              DerefType deref_t)
{
   assert(problemType != PROBLEM_HO); // no need to change derefs
   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if (TermIsFreeVar(s))
   {
      if(s == t)
      {
    return to_equal;
      }
      else
      {
    if(TermIsSubterm(t, s, deref_t))
    {
       return to_lesser;
    }
      }
   }
   else
   {                  /* Note that in this case, t is a variable. */
      if(TermIsSubterm(s, t, deref_s))
      {
    return to_greater;
      }
   }

   return to_uncomparable;
}
