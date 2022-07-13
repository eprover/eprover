/*-----------------------------------------------------------------------

File:    cto_kbo.c

Author:  Stephan Schulz (original Version and some comments by JS)

Contents:
         Implementation of a Knuth_Bendix ordering (KBO) on CLIB
    terms. It is based on the following refined definition:

    Let w:F -> N be a weight function assigning natural numbers
    to operators. Let w be its usual extension to terms. Further,
    let >F be a precedence on F where f is maximal if f is a
    unary operator with weight zero.

    s >KBO t    <=>    Var(s,x) >= Var(t,x)  forall variables x
                       and

                       - s = f^n(x) and t = x where n>=1 and
               w(f)=0  or
             - w(s) > w(t)  or
             - w(s) = w(t) and Head(s) > Head(t)  or
             - w(s) = w(t) and Head(s) ~ Head(t) and
               Args(s) >KBO,lex Args(t)

    where Var(r,x) stands for the number of occurrences of the
    variable x in the term r.


  Copyright 1998, 1999,2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> New
<2> Tue Jun 23 08:36:23 MET DST 1998
    Changed
<3> Fri Aug 17 00:26:53 CEST 2001
    Removed old code


-----------------------------------------------------------------------*/

#include "cto_kbo.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static __inline__ long getweight(OCB_p, FunCode);

static long gettermweight(OCB_p, Term_p, DerefType);

static CompareResult kbocomparevars(Term_p, Term_p,
                DerefType, DerefType);

static CompareResult kbogtrnew(OCB_p, Term_p, Term_p,
             DerefType, DerefType);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: getweight(ocb, op)
//
//   Provides the weight of the operator specified by <op>.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static __inline__ long getweight(OCB_p ocb, FunCode symb)
{
   assert(ocb);
   assert(symb);

   if (symb<0)
   {
      return ocb->var_weight;
   }
   return OCBFunWeight(ocb, symb);
}


/*-----------------------------------------------------------------------
//
// Function: gettermweight(ocb, term)
//
//   Returns the weight of a term t=f(t_1,...,t_n):
//
//                             w(t) = w(f) + w(t_1) + ... + w(t_n)
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

static long gettermweight(OCB_p ocb, Term_p t, DerefType deref)
{
   int i;
   long weight;

   t = TermDeref(t, &deref);

   weight = getweight(ocb, t->f_code);

   if (!(TermIsFreeVar(t)))
   {
      for (i=0; i<t->arity; i++)
      {
    weight += gettermweight(ocb, t->args[i], deref);
      }
   }
   return weight;
}



/*-----------------------------------------------------------------------
//
// Function: kbocomparevars(ocb, s, t)
//
// Compares two terms s and t wrt. KBO if either s or t is a variable
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

static CompareResult kbocomparevars(Term_p s, Term_p t, DerefType
                deref_s, DerefType deref_t)
{
   assert(!t->binding || deref_t == DEREF_NEVER);
   assert(!s->binding || deref_s == DEREF_NEVER);

   if (TermIsFreeVar(t))
   {
      if(s == t)
      {
    return to_equal;
      }
      else
      {
    if (TermIsSubterm(s, t, deref_s))
    {
       return to_greater;
    }
      }
   }
   else
   {               /* Note that in this case, s is a variable. */
      assert(TermIsFreeVar(s));
      if (TermIsSubterm(t, s, deref_t))
      {
    return to_lesser;
      }
   }
   return to_uncomparable;
}


/*-----------------------------------------------------------------------
//
// Function: kbogtrnew(ocb, s, t)
//
//   Returns
//              to_greater         if s >KBO t,
//              to_equal           if s =KBO t,
//              to_uncomparable    otherwise.
//
//   Its a variant of kbogtr where the variable condition is tested in
//   the end.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/
static CompareResult kbogtrnew(OCB_p ocb, Term_p s, Term_p t,
             DerefType deref_s, DerefType deref_t)
{
   int i;
   long sweight, tweight;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if (TermIsFreeVar(s))
   {
      if(s == t)
      {
    return to_equal;
      }
      return to_uncomparable;
   }
   if(TermIsFreeVar(t))
   {
      if(TermIsSubterm(s, t, deref_s))
      {
    return to_greater;
      }
      return to_uncomparable;
   }

   sweight = gettermweight(ocb, s, deref_s);
   tweight = gettermweight(ocb, t, deref_t);

   if(sweight > tweight) {
      if (KBOVarGreater(s, t, deref_s, deref_t)) {
    return to_greater;
      }
      return to_uncomparable;
   }

   if(sweight < tweight) {
      return to_uncomparable;
   }

   assert(sweight == tweight);

   switch (OCBFunCompare(ocb, s->f_code, t->f_code)) {
   case to_greater:
    if (KBOVarGreater(s, t, deref_s, deref_t)) {
       return to_greater;
    }
    return to_uncomparable;
   case to_equal:
    for (i=0; i<MAX(s->arity,t->arity); i++) {
       if (t->arity <= i) {
          if (KBOVarGreater(s, t, deref_s, deref_t)) {
        return to_greater;
          }
          return to_uncomparable;
       }
       if (s->arity <= i) {
          return to_uncomparable;
       }
       switch (kbogtrnew(ocb, s->args[i], t->args[i],
                 deref_s, deref_t)) {
       case to_greater:
        if (KBOVarGreater(s, t, deref_s, deref_t)) {
           return to_greater;
        }
        return to_uncomparable;
       case to_uncomparable:
        return to_uncomparable;
       case to_equal:
         break;
       default:
        assert(false);
        break;
       }
    }
    return to_equal;
   case to_uncomparable:
   case to_lesser:
     return to_uncomparable;
   default:
    assert(false);
    break;
   }
   assert(false);                     /* Should never come here ... */
   return to_uncomparable;
}




/*---------------------------------------------------------------------*/
/*                      Exported Functions                             */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: KBOCompare(ocb, s, t)
//
//   Compare two terms s,t in the Knuth-Bendix Ordering,
//   return the result
//                          to_greater         if s >KBO t
//                          to_equal           if s =KBO t
//                          to_lesser          if t >KBO s
//                          to_uncomparable    otherwise
//
//   Its a variant of KBOCompare where the variable condition is
//   tested in the end.
//   NOTE: derefs have not been updated here because it is used only on
//         FOL terms.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/
CompareResult KBOCompare(OCB_p ocb, Term_p s, Term_p t,
          DerefType deref_s, DerefType deref_t)
{
   assert(problemType != PROBLEM_HO);

   CompareResult topsymb_comp, res;
   int i;
   long sweight, tweight;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   /* The cases s=x v t=x are checked separately. */

   if (TermIsFreeVar(s) || TermIsFreeVar(t))
   {
      return kbocomparevars(s, t, deref_s, deref_t);
   }

   sweight = gettermweight(ocb, s, deref_s);
   tweight = gettermweight(ocb, t, deref_t);

   if (sweight > tweight)
   {
      switch (KBOVarCompare(s, t, deref_s, deref_t))
      {
      case to_greater:
      case to_equal:
       return to_greater;
      case to_uncomparable:
      case to_lesser:
       return to_uncomparable;
      default:
       assert(false);
       return to_uncomparable;
      }
   }

   if (sweight < tweight)
   {
      switch (KBOVarCompare(s, t, deref_s, deref_t))
      {
      case to_lesser:
      case to_equal:
       return to_lesser;
      case to_uncomparable:
      case to_greater:
       return to_uncomparable;
      default:
       assert(false);
       return to_uncomparable;
      }
   }

   assert(sweight == tweight);

   topsymb_comp = OCBFunCompare(ocb, s->f_code, t->f_code);

   switch (topsymb_comp)
   {
   case to_uncomparable:
    return to_uncomparable;
   case to_greater:
    switch (KBOVarCompare(s, t, deref_s, deref_t))
    {
    case to_greater:
    case to_equal:
          return to_greater;
    case to_uncomparable:
    case to_lesser:
          return to_uncomparable;
    default:
          assert(false);
          return to_uncomparable;
    }
   case to_lesser:
    switch (KBOVarCompare(s, t, deref_s, deref_t))
    {
    case to_lesser:
    case to_equal:
          return to_lesser;
    case to_uncomparable:
    case to_greater:
          return to_uncomparable;
    default:
          assert(false);
          return to_uncomparable;
    }
   case to_equal:
    for (i=0; i<MAX(s->arity,t->arity); i++)
    {
       if (t->arity <= i)
       {
          switch (KBOVarCompare(s, t, deref_s, deref_t))
          {
          case to_greater:
          case to_equal:
           return to_greater;
          case to_uncomparable:
          case to_lesser:
           return to_uncomparable;
          default:
           assert(false);
           return to_uncomparable;
          }
       }
       if (s->arity <= i)
       {
          switch (KBOVarCompare(s, t, deref_s, deref_t))
          {
          case to_lesser:
          case to_equal:
           return to_lesser;
          case to_uncomparable:
          case to_greater:
           return to_uncomparable;
          default:
           assert(false);
           return to_uncomparable;
          }
       }
       if ((res = KBOCompare(ocb, s->args[i], t->args[i],
              deref_s, deref_t)) == to_greater)
       {
          switch (KBOVarCompare(s, t, deref_s, deref_t))
          {
          case to_greater:
          case to_equal:
           return to_greater;
          case to_uncomparable:
          case to_lesser:
           return to_uncomparable;
          default:
           assert(false);
           return to_uncomparable;
          }
       }
       if (res == to_lesser)
       {
          switch (KBOVarCompare(s, t, deref_s, deref_t))
          {
          case to_lesser:
          case to_equal:
           return to_lesser;
          case to_uncomparable:
          case to_greater:
           return to_uncomparable;
          default:
           assert(false);
           return to_uncomparable;
          }
       }
       if (res == to_uncomparable)
       {
          return to_uncomparable;
       }
    }
   return to_equal;
   default:
    assert(false);
    return to_uncomparable;
   }
}


/*-----------------------------------------------------------------------
//
// Function: KBOVarCompare()
//
//   Compare the variable occurences in two terms, return the strongest
//   KBO result compatible with the variable condition.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompareResult KBOVarCompare(Term_p s, Term_p t, DerefType deref_s,
             DerefType deref_t)
{
   bool           s_gt = false, t_gt = false;
   VarHash_p      hash = VarHashAlloc();
   int            i;
   VarHashEntry_p handle;

   VarHashAddVarDistrib(hash, s, deref_s, 1);
   VarHashAddVarDistrib(hash, t, deref_t, -1);

   for(i=0; i<VAR_HASH_SIZE; i++)
   {
      for(handle=hash->hash[i]; handle; handle=handle->next)
      {
    if(handle->val > 0)
    {
       s_gt = true;
    }
    else if(handle->val < 0)
    {
       t_gt = true;
    }
      }
      if(s_gt && t_gt)
      {
    break;
      }
   }

   VarHashFree(hash);

   if(s_gt && t_gt)
   {
      return to_uncomparable;
   }
   if(s_gt)
   {
      return to_greater;
   }
   if(t_gt)
   {
      return to_lesser;
   }
   return to_equal;
}


#ifdef NEVER_DEFINED

/*-----------------------------------------------------------------------
//
// Function: KBOVarCompare()
//
//   Compare the variable occurences in two terms, return the strongest
//   KBO result compatible with the variable condition.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompareResult KBOVarCompare(Term_p s, Term_p t, DerefType deref_s,
             DerefType deref_t)
{
   bool           s_gt = false, t_gt = false;
   PDArray_p      array = PDIntArrayAlloc(8,0);
   int            i;

   PDArrayAddVarDistrib(array, s, deref_s, 1);
   PDArrayAddVarDistrib(array, t, deref_t, -1);

   for(i=0; i<array->size; i++)
   {
      if(PDArrayElementInt(array, i) > 0)
      {
    s_gt = true;
      }
      else if(PDArrayElementInt(array, i) < 0)
      {
    t_gt = true;
      }
      if(s_gt && t_gt)
      {
    break;
      }
   }

   PDArrayFree(array);

   if(s_gt && t_gt)
   {
      return to_uncomparable;
   }
   if(s_gt)
   {
      return to_greater;
   }
   if(t_gt)
   {
      return to_lesser;
   }
   return to_equal;
}

#endif


/*-----------------------------------------------------------------------
//
// Function: KBOGreater(ocb, s, t)
//
//   Checks whether the term s is greater than the term t in the
//   Knuth-Bendix Ordering (KBO), i.e. returns
//
//                       true      if s >KBO t,
//                       false     otherwise.
//
//   For a description of the KBO see the header of this file.
//
//   Its a variant of KBOGreater where the variable condition is
//   tested in the end.
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

bool KBOGreater(OCB_p ocb, Term_p s, Term_p t,
      DerefType deref_s, DerefType deref_t)
{
   if (kbogtrnew(ocb, s, t, deref_s, deref_t) == to_greater)
   {
      return true;
   }
   return false;
}

#ifdef NEVER_DEFINED /* I don't really trust this version */
/*-----------------------------------------------------------------------
//
// Function: KBOVarGreater()
//
//   Return true if vars(s) multisetsubseteq vars(t), false otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool KBOVarGreater(Term_p s, Term_p t, DerefType deref_s, DerefType
         deref_t)
{
   bool           t_gt = false;
   PDArray_p      array = PDIntArrayAlloc(8,0);
   int            i;

   PDArrayAddVarDistrib(array, s, deref_s, 1);
   PDArrayAddVarDistrib(array, t, deref_t, -1);

   for(i=0; i<array->size; i++)
   {
      if(PDArrayElementInt(array, i) < 0)
      {
    t_gt = true;
    break;
      }
      if(t_gt)
      {
    break;
      }
   }

   PDArrayFree(array);

   if(t_gt)
   {
      return false;
   }
   return true;
}
#endif
/*-----------------------------------------------------------------------
//
// Function: KBOVarGreater()
//
//   Return true if vars(s) multisetsubseteq vars(t), false otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool KBOVarGreater(Term_p s, Term_p t, DerefType deref_s, DerefType
         deref_t)
{
   bool           t_gt = false;
   VarHash_p      hash = VarHashAlloc();
   int            i;
   VarHashEntry_p handle;

   VarHashAddVarDistrib(hash, s, deref_s, 1);
   VarHashAddVarDistrib(hash, t, deref_t, -1);

   for(i=0; i<VAR_HASH_SIZE; i++)
   {
      for(handle=hash->hash[i]; handle; handle=handle->next)
      {
    if(handle->val < 0)
    {
       t_gt = true;
       break;
    }
      }
      if(t_gt)
      {
    break;
      }
   }

   VarHashFree(hash);

   if(t_gt)
   {
      return false;
   }
   return true;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
