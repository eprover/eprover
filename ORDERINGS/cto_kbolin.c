/*-----------------------------------------------------------------------

File:    cto_kbolin.c

Author:  Stephan Schulz

Contents:
         Implementation of a Knuth_Bendix ordering (KBO) on CLIB
    terms. This is based on CTKBO4-6 from [Loechner:JAR-2006].


  Copyright 2010 by the author.
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

#include "cto_kbolin.h"
#include "clb_plocalstacks.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static CompareResult kbolincmp(OCB_p ocb, Term_p s, Term_p t,
                      DerefType deref_s, DerefType deref_t);

static CompareResult kbo6cmp(OCB_p ocb, Term_p s, Term_p t,
                      DerefType deref_s, DerefType deref_t);

static CompareResult kbo6cmplex(OCB_p ocb, Term_p s, Term_p t,
                         DerefType deref_s, DerefType deref_t);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: resize_vb()
//
//   Enlarge ocb->vb array enough to accomodate index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void __attribute__ ((noinline)) resize_vb(OCB_p ocb, size_t index)
{
   unsigned long old_size = ocb->vb_size;
   int *tmp               = ocb->vb;

   while(ocb->vb_size <= index)
   {
      ocb->vb_size *= 2;
   }
   ocb->vb = SizeMalloc(ocb->vb_size * sizeof(int));
   memcpy(ocb->vb, tmp, old_size * sizeof(int));
   SizeFree(tmp, old_size * sizeof(int));
   for(size_t i = old_size; i<ocb->vb_size; i++)
   {
      ocb->vb[i] = 0;
   }
}


/*-----------------------------------------------------------------------
//
// Function: inc_vb()
//
//   Update all values in ocb when processing var on the
//   LHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void inc_vb(OCB_p ocb, Term_p var)
{
   const size_t index = -var->f_code;

   if(UNLIKELY(index > ocb->max_var))
   {
      if(UNLIKELY(index >= ocb->vb_size))
      {
         resize_vb(ocb, index);
      }
      ocb->max_var = index;
      ocb->vb[index] = 1;
      ocb->pos_bal++;
      ocb->wb += ocb->var_weight;
   }
   else
   {
      const long tmpbal = ocb->vb[index]++;
      ocb->pos_bal += (tmpbal ==  0);
      ocb->neg_bal -= (tmpbal == -1);
      ocb->wb += ocb->var_weight;
   }
}


/*-----------------------------------------------------------------------
//
// Function: dec_vb()
//
//   Update all values in ocb when processing var on the
//   RHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void dec_vb(OCB_p ocb, Term_p var)
{
   const size_t index = -var->f_code;

   if(UNLIKELY(index > ocb->max_var))
   {
      if(UNLIKELY(index >= ocb->vb_size))
      {
         resize_vb(ocb, index);
      }
      ocb->max_var = index;
      ocb->vb[index] = -1;
      ocb->neg_bal++;
      ocb->wb -= ocb->var_weight;
   }
   else
   {
      const long tmpbal = ocb->vb[index]--;
      ocb->neg_bal += (tmpbal == 0);
      ocb->pos_bal -= (tmpbal == 1);
      ocb->wb -= ocb->var_weight;
   }
}


/*-----------------------------------------------------------------------
//
// Function: local_vb_update()
//
//   Perform a local update of ocb according to t (which is not derefed).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void local_vb_update(OCB_p ocb, Term_p t, bool lhs)
{
   if(lhs)
   {
      if(TermIsVar(t))
      {
         inc_vb(ocb, t);
      }
      else
      {
         ocb->wb += OCBFunWeight(ocb, t->f_code);
      }
   }
   else
   {
      if(TermIsVar(t))
      {
         dec_vb(ocb, t);
      }
      else
      {
         ocb->wb -= OCBFunWeight(ocb, t->f_code);
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: mfyvwbc()
//
//   Update ocb according to t and lhs while checking if var occurs in t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool mfyvwbc(OCB_p ocb, Term_p t, DerefType deref_t, Term_p var, bool lhs)
{
   t = TermDeref(t, &deref_t);
   local_vb_update(ocb, t, lhs);

   if(var->f_code == t->f_code)
   {
      return true;
   }

   bool res = false;
   for(size_t i=0; i<t->arity; i++)
   {
      res |= mfyvwbc(ocb, t->args[i], deref_t, var, lhs);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: mfyvwb()
//
//   Update ocb according to t and lhs.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void mfyvwb(OCB_p ocb, Term_p t, DerefType deref_t, bool lhs)
{
   t = TermDeref(t, &deref_t);
   local_vb_update(ocb, t, lhs);

   // Note that arity == 0 for variables.
   for(size_t i=0; i<t->arity; i++)
   {
      mfyvwb(ocb, t->args[i], deref_t, lhs);
   }
}


/*-----------------------------------------------------------------------
//
// Function: kbo6cmplex()
//
//   Perform a lexicographical comparison of the argument lists of s
//   and t, updating the variable/weight balances accordingly.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult kbo6cmplex(OCB_p ocb, Term_p s, Term_p t,
                                DerefType deref_s, DerefType deref_t)
{
   CompareResult res = to_equal;

   assert(s->arity == t->arity);
   assert(s->f_code == t->f_code);

   for(size_t i=0; i<s->arity; i++)
   {
      if(res == to_equal)
      {
         res = kbo6cmp(ocb, s->args[i], t->args[i], deref_s, deref_t);
      }
      else
      {
         mfyvwb(ocb, s->args[i], deref_s, true);
         mfyvwb(ocb, t->args[i], deref_t, false);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: kbo6cmp()
//
//   Perform a KBO comparison between s and t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult kbo6cmp(OCB_p ocb, Term_p s, Term_p t,
                             DerefType deref_s, DerefType deref_t)
{
   CompareResult res, tmp;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   /* Pacman lemma ;-) */
   while((s->arity==1) && (s->f_code == t->f_code))
   {
      s = s->args[0];
      t = t->args[0];
      s = TermDeref(s, &deref_s);
      t = TermDeref(t, &deref_t);
   }
   if(TermIsVar(s))
   {
      if(TermIsVar(t))
      {  /* X, Y */
         inc_vb(ocb, s);
         dec_vb(ocb, t);
         res = s==t?to_equal:to_uncomparable;
      }
      else
      { /* X, t */
         bool ctn = mfyvwbc(ocb, t, deref_t, s, false);
         inc_vb(ocb, s);
         res = ctn?to_lesser:to_uncomparable;
      }
   }
   else if(TermIsVar(t))
   { /* s, Y */
      bool ctn = mfyvwbc(ocb, s, deref_s, t, true);
      dec_vb(ocb, t);
      res = ctn?to_greater:to_uncomparable;
   }
   else
   { /* s, t */
      CompareResult lex;
      if(s->f_code == t->f_code)
      {
         lex = kbo6cmplex(ocb, s, t, deref_s, deref_t);
      }
      else
      {
         lex = to_uncomparable;
         mfyvwb(ocb, s, deref_s, true);
         mfyvwb(ocb, t, deref_t, false);
      }
      CompareResult g_or_n = ocb->neg_bal?to_uncomparable:to_greater;
      CompareResult l_or_n = ocb->pos_bal?to_uncomparable:to_lesser;
      if(ocb->wb>0)
      {
         res = g_or_n;
      }
      else if(ocb->wb<0)
      {
         res = l_or_n;
      }
      else if((tmp = OCBFunCompare(ocb, s->f_code, t->f_code))==to_greater)
      {
         res = g_or_n;
      }
      else if(tmp==to_lesser)
      {
         res = l_or_n;
      }
      else if(s->f_code != t->f_code)
      {
         res = to_uncomparable;
      }
      else if(lex == to_equal)
      {
         res = to_equal;
      }
      else if(lex == to_greater)
      {
         res = g_or_n;
      }
      else if(lex == to_lesser)
      {
         res = l_or_n;
      }
      else
      {
         res = to_uncomparable;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: mfyvwblhs()
//
//   Update ocb according to term on the LHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void mfyvwblhs(OCB_p ocb, Term_p term, DerefType deref_t)
{
   PLocalTaggedStackInit(stack);

   PLocalTaggedStackPush(stack, term, deref_t);

   while(!PLocalTaggedStackEmpty(stack))
   {
      PLocalTaggedStackPop(stack, term, deref_t);
      term = TermDeref(term, &deref_t);

      if(TermIsVar(term))
      {
         inc_vb(ocb, term);
      }
      else
      {
         ocb->wb += OCBFunWeight(ocb, term->f_code);
         PLocalTaggedStackPushTermArgs(stack, term, deref_t);
      }
   }

   PLocalTaggedStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: mfyvwbrhs()
//
//   Update ocb according to term on the RHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void mfyvwbrhs(OCB_p ocb, Term_p term, DerefType deref_t)
{
   PLocalTaggedStackInit(stack);

   PLocalTaggedStackPush(stack, term, deref_t);

   while(!PLocalTaggedStackEmpty(stack))
   {
      PLocalTaggedStackPop(stack, term, deref_t);
      term = TermDeref(term, &deref_t);

      if(TermIsVar(term))
      {
         dec_vb(ocb, term);
      }
      else
      {
         ocb->wb -= OCBFunWeight(ocb, term->f_code);
         PLocalTaggedStackPushTermArgs(stack, term, deref_t);
      }
   }

   PLocalTaggedStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: kbolincmp()
//
//   Perform a KBO comparison between s and t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static CompareResult kbolincmp(OCB_p ocb, Term_p s, Term_p t,
                             DerefType deref_s, DerefType deref_t)
{
   CompareResult res = to_equal;

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if(s->f_code == t->f_code)
   {
      for(int i=0; i<s->arity; i++)
      {
         res = kbolincmp(ocb, s->args[i], t->args[i], deref_s, deref_t);
         if(res!=to_equal)
         {
            i++;
            if(i < s->arity)
            {
               for(;i<s->arity; i++)
               {
                  mfyvwblhs(ocb, s->args[i], deref_s);
                  mfyvwbrhs(ocb, t->args[i], deref_t);
               }

               CompareResult g_or_n = ocb->neg_bal?to_uncomparable:to_greater;
               CompareResult l_or_n = ocb->pos_bal?to_uncomparable:to_lesser;

               if(ocb->wb>0)
               {
                  res = g_or_n;
               }
               else if(ocb->wb<0)
               {
                  res = l_or_n;
               }
               else if(res == to_greater)
               {
                  res = g_or_n;
               }
               else if(res == to_lesser)
               {
                  res = l_or_n;
               }
            }
         }
      }
   }
   else if(TermIsVar(s))
   {
      if(TermIsVar(t))
      {  /* X, Y */
         inc_vb(ocb, s);
         dec_vb(ocb, t);
         res = to_uncomparable;
      }
      else
      { /* X, t */
         inc_vb(ocb, s);
         mfyvwbrhs(ocb, t, deref_t);
         res = ocb->pos_bal?to_uncomparable:to_lesser;
      }
   }
   else if(TermIsVar(t))
   { /* s, Y */
      dec_vb(ocb, t);
      mfyvwblhs(ocb, s, deref_s);
      res = ocb->neg_bal?to_uncomparable:to_greater;
   }
   else
   { /* s, t */
      mfyvwblhs(ocb, s, deref_s);
      mfyvwbrhs(ocb, t, deref_t);
      CompareResult g_or_n = ocb->neg_bal?to_uncomparable:to_greater;
      CompareResult l_or_n = ocb->pos_bal?to_uncomparable:to_lesser;
      if(ocb->wb>0)
      {
         res = g_or_n;
      }
      else if(ocb->wb<0)
      {
         res = l_or_n;
      }
      else
      {
         CompareResult tmp = OCBFunCompare(ocb, s->f_code, t->f_code);
         if(tmp == to_greater)
         {
            res = g_or_n;
         }
         else if(tmp == to_lesser)
         {
            res = l_or_n;
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: kbo6reset()
//
//  Reset data in ocb changed when determining KBO6 comparison of terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void __inline__ kbo6reset(OCB_p ocb)
{
   for(size_t i=0; i<=ocb->max_var; i++)
   {
      ocb->vb[i] = 0;
   }
   ocb->wb      = 0;
   ocb->pos_bal = 0;
   ocb->neg_bal = 0;
   ocb->max_var = 0;
}


/*---------------------------------------------------------------------*/
/*                      Exported Functions                             */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: KBO6Compare(ocb, s, t)
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
//
// Global Variables: -
//
// Side Effects    : -
//
-----------------------------------------------------------------------*/

CompareResult KBO6Compare(OCB_p ocb, Term_p s, Term_p t,
          DerefType deref_s, DerefType deref_t)
{
   CompareResult res;

   kbo6reset(ocb);
   res = kbolincmp(ocb, s, t, deref_s, deref_t);
   assert((kbo6reset(ocb), res == kbo6cmp(ocb, s, t, deref_s, deref_t)));
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: KBO6Greater(ocb, s, t)
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

bool KBO6Greater(OCB_p ocb, Term_p s, Term_p t,
      DerefType deref_s, DerefType deref_t)
{
   CompareResult res;

   kbo6reset(ocb);
   res = kbolincmp(ocb, s, t, deref_s, deref_t);
   assert((kbo6reset(ocb), res == kbo6cmp(ocb, s, t, deref_s, deref_t)));
   return res == to_greater;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
