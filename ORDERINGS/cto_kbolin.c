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

static bool mfyvwbc(OCB_p ocb, Term_p t, DerefType deref_t,
                    Term_p var, bool lhs, int orig_limit)
{
   const int limit_t = orig_limit != 0 ? DEREF_LIMIT(t, deref_t) : orig_limit;
   t = TermDeref(t, &deref_t);
   local_vb_update(ocb, t, lhs);

   if(var->f_code == t->f_code)
   {
      return true;
   }

   bool res = false;
   for(size_t i=0; i<t->arity; i++)
   {
      res |= mfyvwbc(ocb, t->args[i], CONVERT_DEREF(i, limit_t, deref_t),
                     var, lhs, 0);
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

static void mfyvwb(OCB_p ocb, Term_p t, DerefType deref_t,
                   bool lhs, int orig_limit)
{
   const int limit_t = orig_limit != 0 ? DEREF_LIMIT(t, deref_t) : orig_limit;
   t = TermDeref(t, &deref_t);
   local_vb_update(ocb, t, lhs);

   // Note that arity == 0 for variables.
   for(size_t i=0; i<t->arity; i++)
   {
      mfyvwb(ocb, t->args[i], CONVERT_DEREF(i, limit_t, deref_t), lhs, 0);
   }
}


/*-----------------------------------------------------------------------
//
// Function: kbo6cmplex()
//
//   Perform a lexicographical comparison of the argument lists of s
//   and t, updating the variable/weight balances accordingly.
//    NB: function called only for FO terms
//
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult kbo6cmplex(OCB_p ocb, Term_p s, Term_p t,
                                DerefType deref_s, DerefType deref_t)
{
   assert(problemType != PROBLEM_HO);
   CompareResult res = to_equal;

   assert(s->arity == t->arity);
   assert(s->f_code == t->f_code);

   for(size_t i=0; i < MIN(s->arity, t->arity); i++)
   {
      if(res == to_equal)
      {
         res = kbo6cmp(ocb, s->args[i], t->args[i], deref_s, deref_t);
      }
      else
      {
         mfyvwb(ocb, s->args[i], deref_s, true, 0);
         mfyvwb(ocb, t->args[i], deref_t, false, 0);
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
   assert(problemType != PROBLEM_HO); // thus, no need to change derefs
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
         bool ctn = mfyvwbc(ocb, t, deref_t, s, false, 0);
         inc_vb(ocb, s);
         res = ctn?to_lesser:to_uncomparable;
      }
   }
   else if(TermIsVar(t))
   { /* s, Y */
      bool ctn = mfyvwbc(ocb, s, deref_s, t, true, 0);
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
         mfyvwb(ocb, s, deref_s, true, 0);
         mfyvwb(ocb, t, deref_t, false, 0);
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

static void mfyvwblhs(OCB_p ocb, Term_p term, DerefType deref_t, int orig_limit)
{
   const Term_p orig_term = term;
   PLocalTaggedStackInit(stack);

   PLocalTaggedStackPush(stack, term, deref_t);

   while(!PLocalTaggedStackEmpty(stack))
   {
      PLocalTaggedStackPop(stack, term, deref_t);
      int limit_t;
      if(term == orig_term && orig_limit != 0)
      {
         limit_t = orig_limit;
      }
      else
      {
         limit_t = DEREF_LIMIT(term, deref_t);
         term = TermDeref(term, &deref_t);
      }

      if(TermIsVar(term))
      {
         inc_vb(ocb, term);
      }
      else
      {
         ocb->wb += OCBFunWeight(ocb, term->f_code);
         PLocalTaggedStackPushTermArgs(stack, term, CONVERT_DEREF(i, limit_t, deref_t));
         //.......................................................i in macro expansion
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
/----------------------------------------------ss------------------------*/

static void mfyvwbrhs(OCB_p ocb, Term_p term, DerefType deref_t, int orig_limit)
{
   const Term_p orig_term = term;
   PLocalTaggedStackInit(stack);

   PLocalTaggedStackPush(stack, term, deref_t);

   while(!PLocalTaggedStackEmpty(stack))
   {
      PLocalTaggedStackPop(stack, term, deref_t);
      int limit_t;
      if(term == orig_term && orig_limit != 0)
      {
         limit_t = orig_limit;
      }
      else
      {
         limit_t = DEREF_LIMIT(term, deref_t);
         term = TermDeref(term, &deref_t);
      }

      if(TermIsVar(term))
      {
         dec_vb(ocb, term);
      }
      else
      {
         ocb->wb -= OCBFunWeight(ocb, term->f_code);
         PLocalTaggedStackPushTermArgs(stack, term, CONVERT_DEREF(i, limit_t, deref_t));
         //.......................................................i in macro expansion
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
   assert(problemType != PROBLEM_HO); // no need to change derefs
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
                  mfyvwblhs(ocb, s->args[i], deref_s, 0);
                  mfyvwbrhs(ocb, t->args[i], deref_t, 0);
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
         mfyvwbrhs(ocb, t, deref_t, 0);
         res = ocb->pos_bal?to_uncomparable:to_lesser;
      }
   }
   else if(TermIsVar(t))
   { /* s, Y */
      dec_vb(ocb, t);
      mfyvwblhs(ocb, s, deref_s, 0);
      res = ocb->neg_bal?to_uncomparable:to_greater;
   }
   else
   { /* s, t */
      mfyvwblhs(ocb, s, deref_s, 0);
      mfyvwbrhs(ocb, t, deref_t, 0);
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

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: cmp_arities()
//
//   Support length-lexicographic comparsion.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult cmp_arities(Term_p s, Term_p t)
{
   assert(s->arity != t->arity);
   return s->arity > t->arity ? to_greater : to_lesser;
}


/*-----------------------------------------------------------------------
//
// Function: kbolincmp_ho()
//
//   Perform a KBO comparison between s and t, which are LFHOL terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static CompareResult kbolincmp_ho(OCB_p ocb, Term_p s, Term_p t,
                             DerefType deref_s, DerefType deref_t)
{
   assert(problemType == PROBLEM_HO);
   CompareResult res = to_equal;

   const int limit_s = DEREF_LIMIT(s, deref_s);
   const int limit_t = DEREF_LIMIT(t, deref_t);

   s = TermDeref(s, &deref_s);
   t = TermDeref(t, &deref_t);

   if(s->f_code == t->f_code)
   {
      // if we have two constants of the same fun code, there's nothing to update.
      bool done = s->arity == t->arity ? s->arity == 0 : false;
      int i = 0;
      while(!done)
      {
         res = s->arity == t->arity ? kbolincmp_ho(ocb, s->args[i], t->args[i],
                                                   CONVERT_DEREF(i, limit_s, deref_s),
                                                   CONVERT_DEREF(i, limit_t, deref_t)) :
                                      cmp_arities(s,t);

         if(res!=to_equal)
         {
            // increase only if we got here through kbolincmp_ho
            i += s->arity == t->arity ? 1 : 0;
            if(i < s->arity || i < t->arity)
            {
               for(int j=i;j<s->arity; j++)
               {
                  mfyvwblhs(ocb, s->args[j], CONVERT_DEREF(j, limit_s, deref_s), 0);
               }

               for(int j=i; j<t->arity; j++)
               {
                  mfyvwbrhs(ocb, t->args[j], CONVERT_DEREF(j, limit_t, deref_t), 0);
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
            done = true;
         }
         else
         {
            assert(t->arity == s->arity);
            i++;
            done = i == s->arity;
         }
      }
   }
   else if(TermIsVar(s))
   {
      if(TermIsVar(t))
      {  /* X, Y */
         inc_vb(ocb, s);
         dec_vb(ocb, t);
         res = t == s ? to_equal : to_uncomparable;
      }
      else
      { /* X, t */
         inc_vb(ocb, s);
         mfyvwbrhs(ocb, t, deref_t, limit_t);
         res = ocb->pos_bal?to_uncomparable:to_lesser;
      }
   }
   else if(TermIsVar(t))
   { /* s, Y */
      dec_vb(ocb, t);
      mfyvwblhs(ocb, s, deref_s, limit_s);
      res = ocb->neg_bal?to_uncomparable:to_greater;
   }
   else
   { /* s, t */
      mfyvwblhs(ocb, s, deref_s, limit_s);
      mfyvwbrhs(ocb, t, deref_t, limit_t);
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
         assert(!TermIsAppliedVar(s) || !TermIsAppliedVar(t));
         CompareResult tmp = (TermIsAppliedVar(s) || TermIsAppliedVar(t)) ?
                               to_uncomparable : OCBFunCompare(ocb, s->f_code, t->f_code);
         if(tmp == to_greater)
         {
            res = g_or_n;
         }
         else if(tmp == to_lesser)
         {
            res = l_or_n;
         }
         else
         {
            assert(tmp == to_uncomparable);
            res = to_uncomparable;
         }
      }
   }
   return res;
}
#endif

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

#ifdef ENABLE_LFHO
   res = problemType == PROBLEM_HO ?
            kbolincmp_ho(ocb, s, t, deref_s, deref_t)
            : kbolincmp(ocb, s, t, deref_s, deref_t);
   //res = kbolincmp(ocb, s, t, deref_s, deref_t);
#else
   res = kbolincmp(ocb, s, t, deref_s, deref_t);
   assert((kbo6reset(ocb), res == kbo6cmp(ocb, s, t, deref_s, deref_t)));
#endif

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
#ifdef ENABLE_LFHO
   res = problemType == PROBLEM_HO ?
            kbolincmp_ho(ocb, s, t, deref_s, deref_t)
            : kbolincmp(ocb, s, t, deref_s, deref_t);
#else
   res = kbolincmp(ocb, s, t, deref_s, deref_t);
   assert((kbo6reset(ocb), res == kbo6cmp(ocb, s, t, deref_s, deref_t)));
#endif

   return res == to_greater;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
