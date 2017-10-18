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

  Copyright 1998, 1999,2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
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
         <6> Thu Apr 22 23:14:52 CEST 2004
             Started implementing the polynomial LPO4 algorithm from
             Bernd Loechners paper "What to know about LPO"

-----------------------------------------------------------------------*/

#include "cto_lpo.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long LPORecursionDepthLimit = 1000;


/*---------------------------------------------------------------------*/
/*                     Forward Declarations LPO                        */
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
/*                      Internal Functions LPO                         */
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
      /* printf("LPORecursionDepthLimit encountered\n"); */
      return to_uncomparable;
   }
   recursion_depth++;

   if(TermIsVar(s))
   {
      if(s == t)
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
      if(TermIsSubterm(s, t, deref_s))
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
/*             Forward Declarations LPO4 on context terms              */
/*---------------------------------------------------------------------*/


static bool lpo4_alpha(OCB_p ocb, Term_p s, int pos, Term_p t,
                       DerefType deref_s, DerefType deref_t);
static bool lpo4_majo(OCB_p ocb, Term_p s, Term_p t, int pos,
                      DerefType deref_s, DerefType deref_t);
static bool lpo4_lex_ma(OCB_p ocb, Term_p s, Term_p t, int pos,
                        DerefType deref_s, DerefType deref_t);
static bool lpo4_greater(OCB_p ocb, Term_p s, Term_p t,
                         DerefType deref_s, DerefType deref_t);


/*---------------------------------------------------------------------*/
/*                      Internal Functions LPO4                        */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: lpo4_alpha()
//
//   Handle the LPO case alpha (s_i >=LPO t). s, pos represents the
//   argument list of s starting at pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_alpha(OCB_p ocb, Term_p s, int pos, Term_p t,
                       DerefType deref_s, DerefType deref_t)
{
   for(/*Nothing*/;pos < s->arity;pos++)
   {
      if(TermStructEqualDeref(s->args[pos], t, deref_s, deref_t)
          ||
          lpo4_greater(ocb, s->args[pos], t, deref_s, deref_t))
      {
         return true;
      }
   }
   return false;
}


/*-----------------------------------------------------------------------
//
// Function: lpo4_majo()
//
//   Handle the majorisation check of LPO (s >=LPO t_i for all i). See
//   above (this time its t, pos).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_majo(OCB_p ocb, Term_p s, Term_p t, int pos,
                      DerefType deref_s, DerefType deref_t)
{
   for(/*Nothing*/;pos < t->arity;pos++)
   {
      if(!lpo4_greater(ocb, s, t->args[pos], deref_s, deref_t))
      {
         return false;
      }
   }
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: lpo4_lex_ma()
//
//   Implement the lex_ma_4 function, combining lexicographical
//   comparison and alpha case.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_lex_ma(OCB_p ocb, Term_p s, Term_p t, int pos,
                        DerefType deref_s, DerefType deref_t)
{
   assert(s->f_code == t->f_code);

   for(/* Nothing */; pos<s->arity; pos++)
   {
      if(pos >= t->arity) /* s->args >_lex t->args */
      {
         return true;
      }
      if(TermStructEqualDeref(s->args[pos], t->args[pos], deref_s, deref_t))
      {
         /* Next argument */
         continue;
      }
      if(lpo4_greater(ocb, s->args[pos], t->args[pos], deref_s, deref_t))
      {
         return lpo4_majo(ocb, s,t,pos+1, deref_s, deref_t);
      }
      return lpo4_alpha(ocb, s, pos+1, t, deref_s, deref_t);
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: lpo4_greater()
//
//   LPO comparison using the lpo_4_nc algorithm by Bernd Loechner.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static bool lpo4_greater(OCB_p ocb, Term_p s, Term_p t,
                         DerefType deref_s, DerefType deref_t)
{
   static long   recursion_depth = 0;
   CompareResult f_code_res;
   bool res;

   if(recursion_depth > LPORecursionDepthLimit)
   {
      return false;
   }
   recursion_depth++;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if(TermIsVar(s))
   {
      res = false;
   }
   else if(TermIsVar(t))
   {
      res = TermIsSubterm(s, t, deref_s);
   }
   else
   {
      f_code_res = OCBFunCompare(ocb, s->f_code, t->f_code);
      if(f_code_res==to_greater)
      {
         res = lpo4_majo(ocb, s, t, 0, deref_s, deref_t);
      }
      else if(f_code_res==to_equal)
      {
         res = lpo4_lex_ma(ocb, s, t, 0, deref_s, deref_t);
      }
      else
      {
         res = lpo4_alpha(ocb, s, 0, t, deref_s, deref_t);
      }
   }
   recursion_depth--;
   return res;
}




/*---------------------------------------------------------------------*/
/*             Forward Declarations LPO4 on static terms               */
/*---------------------------------------------------------------------*/


static bool lpo4_copy_alpha(OCB_p ocb, Term_p s, int pos, Term_p t);
static bool lpo4_copy_majo(OCB_p ocb, Term_p s, Term_p t, int pos);
static bool lpo4_copy_lex_ma(OCB_p ocb, Term_p s, Term_p t, int pos);
static bool lpo4_copy_greater(OCB_p ocb, Term_p s, Term_p t);

/*---------------------------------------------------------------------*/
/*                      Internal Functions LPO4                        */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: lpo4_copy_alpha()
//
//   Handle the LPO case alpha (s_i >=LPO t). s, pos represents the
//   argument list of s starting at pos.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_copy_alpha(OCB_p ocb, Term_p s, int pos, Term_p t)
{
   if(pos == s->arity)
   {
      return false;
   }
   return TermStructEqualNoDeref(s->args[pos], t)
      ||
      lpo4_copy_greater(ocb, s->args[pos], t)
      ||
      lpo4_copy_alpha(ocb, s,pos+1, t);
}


/*-----------------------------------------------------------------------
//
// Function: lpo4_copy_majo()
//
//   Handle the majorisation check of LPO (s >=LPO t_i for all i). See
//   above (this time its t, pos).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_copy_majo(OCB_p ocb, Term_p s, Term_p t, int pos)
{
   if(pos == t->arity)
   {
      return true;
   }
   return lpo4_copy_greater(ocb, s, t->args[pos])
      &&
      lpo4_copy_majo(ocb, s, t, pos+1);
}


/*-----------------------------------------------------------------------
//
// Function: lpo4_copy_lex_ma()
//
//   Implement the lex_ma_4_nc function, combining lexicographical
//   comparison and alpha case.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_copy_lex_ma(OCB_p ocb, Term_p s, Term_p t, int pos)
{
   assert(s->f_code == t->f_code);

   if(pos == s->arity)
   {
      return false;
   }
   if(TermStructEqualNoDeref(s->args[pos], t->args[pos]))
   {
      return lpo4_copy_lex_ma(ocb, s, t, pos+1);
   }
   if(lpo4_copy_greater(ocb, s->args[pos], t->args[pos]))
   {
      return lpo4_copy_majo(ocb, s,t,pos+1);
   }
   return lpo4_copy_alpha(ocb, s, pos+1, t);
}


/*-----------------------------------------------------------------------
//
// Function: lpo4_copy_greater()
//
//   LPO comparison using the lpo_4_nc algorithm by Bernd Loechner.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool lpo4_copy_greater(OCB_p ocb, Term_p s, Term_p t)
{
   CompareResult f_code_res;

   if(TermIsVar(s))
   {
      return false;
   }
   if(TermIsVar(t))
   {
      return TBTermIsSubterm(s, t);
   }
   f_code_res = OCBFunCompare(ocb, s->f_code, t->f_code);
   if(f_code_res==to_greater)
   {
      return lpo4_copy_majo(ocb, s, t, 0);
   }
   if(f_code_res==to_equal)
   {
      return lpo4_copy_lex_ma(ocb, s, t, 0);
   }
   return lpo4_copy_alpha(ocb, s, 0, t);
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
//                       false     otherwise.
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


/*-----------------------------------------------------------------------
//
// Function: LPO4Greater(ocb, s, t, ds, dt)
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

bool LPO4Greater(OCB_p ocb, Term_p s, Term_p t,
      DerefType deref_s, DerefType deref_t)
{
   bool res;

   /* printf("LPO4Greater()...\n"); */
   res =  lpo4_greater(ocb, s, t, deref_s, deref_t);
   /* printf("...LPO4Greater()=%d\n", res); */
   /* assert(res == LPOGreater(ocb, s, t, deref_s, deref_t)); */
  return res;
}


/*-----------------------------------------------------------------------
//
// Function: LPO4Compare()
//
//   Determine relationship between s and t.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompareResult LPO4Compare(OCB_p ocb, Term_p s, Term_p t,
                          DerefType deref_s, DerefType deref_t)
{
   CompareResult res;

   /* printf("LPO4Compare()...\n"); */

   if(TermStructEqualDeref(s, t, deref_s, deref_t))
   {
      res = to_equal;
   }
   else if(lpo4_greater(ocb, s, t, deref_s, deref_t))
   {
      res = to_greater;
   }
   else if(lpo4_greater(ocb, t,s, deref_t, deref_s))
   {
      res = to_lesser;
   }
   else
   {
      res = to_uncomparable;
   }
   /* printf("...LPO4Compare()=%d\n", res); */
   /* assert(res == LPOCompare(ocb, s, t, deref_s, deref_t)); */
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LPO4GreaterCopy()
//
//   Wrapper for comparing two terms using the LPO4 implementation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool LPO4GreaterCopy(OCB_p ocb, Term_p s, Term_p t,
                     DerefType deref_s, DerefType deref_t)
{
   bool res;
   Term_p s1, t1;

   /* printf("LPO4GreaterCopy()...\n"); */

   if(deref_s == DEREF_NEVER)
   {
      s1 = s;
   }
   else
   {
      s1 = TermCopyKeepVars(s, deref_s);
   }
   if(deref_t == DEREF_NEVER)
   {
      t1 = t;
   }
   else
   {
      t1 = TermCopyKeepVars(t, deref_t);
   }

   res = lpo4_copy_greater(ocb, s1,t1);

   if(deref_s != DEREF_NEVER)
   {
      TermFree(s1);
   }
   if(deref_t != DEREF_NEVER)
   {
      TermFree(t1);
   }
   /* printf("...LPO4GreaterCopy()\n");  */
   assert(res == LPOGreater(ocb, s, t, deref_s, deref_t));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LPO4CompareCopy()
//
//   Determine relationship between s and t.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompareResult LPO4CompareCopy(OCB_p ocb, Term_p s, Term_p t,
                              DerefType deref_s, DerefType deref_t)
{
   CompareResult res;
   Term_p s1, t1;

   /* printf("LPO4CompareCopy()...\n"); */

   if(deref_s == DEREF_NEVER)
   {
      s1 = s;
   }
   else
   {
      s1 = TermCopyKeepVars(s, deref_s);
   }
   if(deref_t == DEREF_NEVER)
   {
      t1 = t;
   }
   else
   {
      t1 = TermCopyKeepVars(t, deref_t);
   }
   if(TermStructEqualNoDeref(s1,t1))
   {
      res = to_equal;
   }
   else if(lpo4_copy_greater(ocb, s1,t1))
   {
      res = to_greater;
   }
   else if(lpo4_copy_greater(ocb, t1,s1))
   {
      res = to_lesser;
   }
   else
   {
      res = to_uncomparable;
   }
   if(deref_s != DEREF_NEVER)
   {
      TermFree(s1);
   }
   if(deref_t != DEREF_NEVER)
   {
      TermFree(t1);
   }
   /* printf("...LPO4CompareCopy()=%d\n", res);*/
   assert(res == LPOCompare(ocb, s, t, deref_s, deref_t));
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: LPOGreaterCopy()
//
//   Wrapper for comparing two terms using the standard LPO
//   implementation with uninstantiated terms.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool LPOGreaterCopy(OCB_p ocb, Term_p s, Term_p t,
                    DerefType deref_s, DerefType deref_t)
{
   bool res;
   Term_p s1, t1;

   /* printf("LPOGreaterCopy()...\n"); */

   if(deref_s == DEREF_NEVER)
   {
      s1 = s;
   }
   else
   {
      s1 = TermCopyKeepVars(s, deref_s);
   }
   if(deref_t == DEREF_NEVER)
   {
      t1 = t;
   }
   else
   {
      t1 = TermCopyKeepVars(t, deref_t);
   }

   res = LPOGreater(ocb, s1,t1,DEREF_NEVER,DEREF_NEVER);

   if(deref_s != DEREF_NEVER)
   {
      TermFree(s1);
   }
   if(deref_t != DEREF_NEVER)
   {
      TermFree(t1);
   }
   /* printf("...LPOGreaterCopy()\n"); */
   assert(res == LPOGreater(ocb, s, t, deref_s, deref_t));
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: LPOCompareCopy()
//
//   Determine relationship between s and t.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompareResult LPOCompareCopy(OCB_p ocb, Term_p s, Term_p t,
                             DerefType deref_s, DerefType deref_t)
{
   CompareResult res;
   Term_p s1, t1;

   /* printf("LPOCompareCopy()...\n"); */

   if(deref_s == DEREF_NEVER)
   {
      s1 = s;
   }
   else
   {
      s1 = TermCopyKeepVars(s, deref_s);
   }
   if(deref_t == DEREF_NEVER)
   {
      t1 = t;
   }
   else
   {
      t1 = TermCopyKeepVars(t, deref_t);
   }
   res = LPOCompare(ocb, s1, t1, DEREF_NEVER, DEREF_NEVER);
   if(deref_s != DEREF_NEVER)
   {
      TermFree(s1);
   }
   if(deref_t != DEREF_NEVER)
   {
      TermFree(t1);
   }
   /* printf("...LPOCompareCopy()=%d\n", res); */
   assert(res == LPOCompare(ocb, s, t, deref_s, deref_t));
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
