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
#include <cte_lambda.h>


typedef enum {
   LHS, RHS
} ComparisonSide;

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

#ifdef ENABLE_LFHO
static CompareResult cmp_arities(Term_p s, Term_p t);
#endif


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

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: is_fluid()
//
//   Approximation the fluidity test -- see
//   https://arxiv.org/abs/2102.00453 for definition
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool is_fluid(Term_p t)
{
   return TermIsTopLevelFreeVar(t) ||
          (TermIsLambda(t) && !TermIsGround(t));
}

#endif

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


#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: inc_vb_ho()
//
//   Like inc_vb, but maps fluid terms to fresh variables.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void inc_vb_ho(OCB_p ocb, Term_p var)
{
   assert(is_fluid(var));
   long** bal_ref = (long**)PObjMapGetRef(&ocb->ho_vb, var, PCmpFun, NULL);
   if(*bal_ref == NULL)
   {
      MK_HO_VB_KEY(*bal_ref, 0L);
   }
   ocb->pos_bal += (**bal_ref ==  0);
   ocb->neg_bal -= (**bal_ref == -1);
   **bal_ref += 1;
   ocb->wb += ocb->var_weight;
}

#endif

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

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: dec_vb_ho()
//
//   Like dec_vb, but maps fluid terms to fresh variables.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void dec_vb_ho(OCB_p ocb, Term_p var)
{
   assert(is_fluid(var));
   long** bal_ref = (long**)PObjMapGetRef(&ocb->ho_vb, var, PCmpFun, NULL);
   if(*bal_ref == NULL)
   {
      MK_HO_VB_KEY(*bal_ref, 0L);
   }
   ocb->neg_bal += (**bal_ref == 0);
   ocb->pos_bal -= (**bal_ref == 1);
   **bal_ref -= 1;
   ocb->wb -= ocb->var_weight;
}

#endif

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
      if(TermIsFreeVar(t))
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
      if(TermIsFreeVar(t))
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
   if(TermIsFreeVar(s))
   {
      if(TermIsFreeVar(t))
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
   else if(TermIsFreeVar(t))
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
         term = deref_t == DEREF_ALWAYS && problemType == PROBLEM_HO ?
                WHNF_deref(term) : TermDeref(term, &deref_t);
      }

      if(TermIsFreeVar(term))
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
         term = deref_t == DEREF_ALWAYS && problemType == PROBLEM_HO ?
                WHNF_deref(term) : TermDeref(term, &deref_t);
      }

      if(TermIsFreeVar(term))
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

#ifdef ENABLE_LFHO

/*-----------------------------------------------------------------------
//
// Function: mfyvwblhs_ho()
//
//   Update ocb according to term on the LHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void mfyvwb_ho(OCB_p ocb, Term_p orig, ComparisonSide side)
{
   PLocalStackInit(stack);

   PLocalStackPush(stack, orig);

   while(!PLocalStackEmpty(stack))
   {
      Term_p t = PLocalStackPop(stack);
      if(is_fluid(t))
      {
         (side == LHS ? inc_vb_ho : dec_vb_ho)(ocb, t);
      }
      else
      {
         if(TermIsLambda(t))
         {
            ocb->wb +=
               ((side == LHS) ? OCBLamWeight(ocb) : -OCBLamWeight(ocb));
         }
         else if(TermIsDBVar(t))
         {
            ocb->wb +=
               ((side == LHS) ? OCBDBWeight(ocb) : -OCBDBWeight(ocb));
         }
         else if (!TermIsPhonyApp(t))
         {
            ocb->wb +=
               ((side == LHS) ? OCBFunWeight(ocb, t->f_code)
                            : -OCBFunWeight(ocb, t->f_code));
         }
         PLocalStackEnsureSpace(stack, (t->arity - (TermIsLambda(t) ? 1 : 0)));
         for(int i=TermIsLambda(t) ? 1 : 0; i<t->arity; i++)
         {
            PLocalStackPush(stack, t->args[i]);
         }
      }
   }
   PLocalStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: heads_same()
//
//   Update ocb according to term on the LHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline bool heads_same(Term_p s, Term_p t)
{
   if(!TermIsPhonyApp(s) && !TermIsPhonyApp(t))
   {
      return s->f_code == t->f_code;
   }
   else
   {
      return TermIsPhonyApp(s) && TermIsPhonyApp(t) &&
             s->args[0] == t->args[0];
   }
}



/*-----------------------------------------------------------------------
//
// Function: classify_head()
//
//   Assigns a number to the term head that can be used for comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline int classify_head(Term_p s)
{
   assert(!TermIsTopLevelFreeVar(s));
   // lam > db > symbol
   if(TermIsLambda(s))
   {
      return 3;
   }
   else if(TermIsPhonyApp(s) || TermIsDBVar(s))
   {
      return 2;
   }
   else
   {
      return 1;
   }
}


/*-----------------------------------------------------------------------
//
// Function: cmp_heads()
//
//   Compares head terms.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult cmp_heads(OCB_p ocb, Term_p s, Term_p t)
{
   assert(!TermIsTopLevelFreeVar(s));
   assert(!TermIsTopLevelFreeVar(t));

   int s_hd_class = classify_head(s);
   int t_hd_class = classify_head(t);
   if(s_hd_class == t_hd_class)
   {
      if(!TermIsTopLevelAnyVar(s) && !TermIsLambda(s))
      {
         assert(!TermIsTopLevelAnyVar(t) && !TermIsLambda(t));
         return OCBFunCompare(ocb, s->f_code, t->f_code);
      }
      else if(TermIsTopLevelAnyVar(s))
      {
         assert(TermIsTopLevelAnyVar(t));
         FunCode db_s = (TermIsAppliedAnyVar(s) ? s->args[0] : s)->f_code;
         FunCode db_t = (TermIsAppliedAnyVar(t) ? t->args[0] : t)->f_code;
         assert(db_s != db_t); // KBO should check for equality before
         return (db_s > db_t) ? to_greater : to_lesser;
      }
      else
      {
         return to_uncomparable;
      }
   }
   else
   {
      return (s_hd_class > t_hd_class) ? to_greater : to_lesser;
   }
}

#endif

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
   else if(TermIsFreeVar(s))
   {
      if(TermIsFreeVar(t))
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
   else if(TermIsFreeVar(t))
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
// Function: kbolincmp_lambda_driver()
//
//   Does the actual comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult kbolincmp_lambda_driver(OCB_p ocb, Term_p s, Term_p t)
{
   CompareResult res = to_equal;
   if(is_fluid(s))
   {
      if(is_fluid(t))
      {
         // FLUID, FLUID
         inc_vb_ho(ocb, s);
         dec_vb_ho(ocb, t);
         res = s==t ? to_equal : to_uncomparable;
      }
      else
      {
         // FLUID, t
         inc_vb_ho(ocb, s);
         mfyvwb_ho(ocb, t, RHS);
         res = ocb->pos_bal?to_uncomparable:to_lesser;
      }
   }
   else if(is_fluid(t))
   {
      // s, FLUID
      dec_vb_ho(ocb, t);
      mfyvwb_ho(ocb, s, LHS);
      res = ocb->neg_bal?to_uncomparable:to_greater;
   }
   else
   {
      if(heads_same(s,t))
      {
         bool done = s->arity == t->arity ? s->arity == 0 : false;
         int i = 0;
         while(!done)
         {
            // if one of the terms is applied DB var and the other one is unapplied,
            // arity must be different and the code will jump into length-lexicographic
            res = s->arity == t->arity ? kbolincmp_lambda_driver(ocb, s->args[i], t->args[i]) :
                                         cmp_arities(s,t);

            if(res!=to_equal)
            {
               // increase only if we got here through kbolincmp_ho
               i += s->arity == t->arity ? 1 : 0;
               if(i < s->arity || i < t->arity)
               {
                  for(int j= i==0 && (TermIsPhonyApp(s) || TermIsLambda(s)) ? 1 : i; j<s->arity; j++)
                  {
                     mfyvwb_ho(ocb, s->args[j], LHS);
                  }

                  for(int j= i==0 && (TermIsPhonyApp(t) || TermIsLambda(t)) ? 1 : i; j<t->arity; j++)
                  {
                     mfyvwb_ho(ocb, t->args[j], RHS);
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
      else
      {
         mfyvwb_ho(ocb, s, LHS);
         mfyvwb_ho(ocb, t, RHS);
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
            CompareResult tmp = cmp_heads(ocb, s, t);
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
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: kbolincmp_lambda()
//
//   Perform a KBO comparison between s and t that takes lambdas into
//   account. Amounts to Boolean free derived lambda KBO.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


static CompareResult kbolincmp_lambda(OCB_p ocb, Term_p s, Term_p t,
                                      DerefType deref_s, DerefType deref_t)
{
   assert(problemType == PROBLEM_HO);
   assert(ocb->ho_vb == NULL);
   s = LambdaEtaReduceDB(TermGetBank(s),
         BetaNormalizeDB(TermGetBank(s),
            TBInsertInstantiatedDeref(TermGetBank(s), s, deref_s)));
   t =  LambdaEtaReduceDB(TermGetBank(t),
         BetaNormalizeDB(TermGetBank(t),
            TBInsertInstantiatedDeref(TermGetBank(t), t, deref_t)));

   CompareResult res =
      s->f_code == SIG_TRUE_CODE ?
         (t->f_code == SIG_TRUE_CODE ? to_equal : to_lesser) :
      (t->f_code == SIG_TRUE_CODE ? to_greater : kbolincmp_lambda_driver(ocb, s, t));

   return res;
}


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

static inline CompareResult cmp_arities(Term_p s, Term_p t)
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

   s = deref_s == DEREF_ALWAYS ? WHNF_deref(s) : TermDeref(s, &deref_s);
   t = deref_t == DEREF_ALWAYS ? WHNF_deref(t) : TermDeref(t, &deref_t);

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
   else if(TermIsFreeVar(s))
   {
      if(TermIsFreeVar(t))
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
   else if(TermIsFreeVar(t))
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
         assert(!TermIsAppliedFreeVar(s) || !TermIsAppliedFreeVar(t));
         CompareResult tmp = (TermIsTopLevelAnyVar(s) || TermIsTopLevelAnyVar(t)) ?
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
#ifdef ENABLE_LFHO
   if(ocb->ho_order_kind == LAMBDA_ORDER)
   {
      OCBResetHOVarMap(ocb);
   }
   else
#endif
   {
      for(size_t i=0; i<=ocb->max_var; i++)
      {
         ocb->vb[i] = 0;
      }
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
   assert(ocb->ho_vb == NULL);

#ifdef ENABLE_LFHO
   if(problemType == PROBLEM_HO)
   {
      if(ocb->ho_order_kind == LFHO_ORDER)
      {
         res = kbolincmp_ho(ocb, s, t, deref_s, deref_t);
      }
      else
      {
         res = kbolincmp_lambda(ocb, s, t, deref_s, deref_t);
      }
   }
   else
   {
      res = kbolincmp(ocb, s, t, deref_s, deref_t);
   }
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
            (ocb->ho_order_kind == LFHO_ORDER ? kbolincmp_ho : kbolincmp_lambda)(ocb, s, t, deref_s, deref_t)
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
