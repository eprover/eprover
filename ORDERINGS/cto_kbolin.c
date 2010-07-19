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

#include "cto_kbo.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

static CompareResult kbo6cmp(OCB_p ocb, Term_p s, Term_p t,
                      DerefType deref_s, DerefType deref_t);

static CompareResult kbo6cmplex(OCB_p ocb, Term_p s, Term_p t,
                         DerefType deref_s, DerefType deref_t);

static void mfyvwb(OCB_p ocb, Term_p t, DerefType deref_t, bool lhs);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: inc_vb()
//
//   Update all values in ocb->kbobalance when processing var on the
//   LHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void inc_vb(OCB_p ocb, Term_p var)
{
   long tmpbal;
   long index = -var->f_code;

   if(index>ocb->kbobalance->max_var)
   {
      ocb->kbobalance->max_var = index;
   }
   ocb->kbobalance->wb += ocb->var_weight;
   
   tmpbal = PDArrayElementInt(ocb->kbobalance->vb, index);
   if(tmpbal == 0)
   {
      ocb->kbobalance->pos_bal++;
   }
   else if(tmpbal == -1)
   {
      ocb->kbobalance->neg_bal--;
   }
   tmpbal ++;
   PDArrayAssignInt(ocb->kbobalance->vb, index, tmpbal);
}


/*-----------------------------------------------------------------------
//
// Function: dec_vb()
//
//   Update all values in ocb->kbobalance when processing var on the
//   RHS of a comparison.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void dec_vb(OCB_p ocb, Term_p var)
{
   long tmpbal;
   long index = -var->f_code;

   if(index>ocb->kbobalance->max_var)
   {
      ocb->kbobalance->max_var = index;
   }
   ocb->kbobalance->wb -= ocb->var_weight;
   
   tmpbal = PDArrayElementInt(ocb->kbobalance->vb, index);
   if(tmpbal == 0)
   {
      ocb->kbobalance->neg_bal++;
   }
   else if(tmpbal == 1)
   {
      ocb->kbobalance->pos_bal--;
   }
   tmpbal --;
   PDArrayAssignInt(ocb->kbobalance->vb, index, tmpbal);
}


/*-----------------------------------------------------------------------
//
// Function: local_vb_update()
//
//   Perform a local update of ocb->kbodata according to t (which is
//   not derefed).
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
         ocb->kbobalance->wb += OCBFunWeight(ocb, t->f_code);
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
         ocb->kbobalance->wb -= OCBFunWeight(ocb, t->f_code);
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: mfyvwbc()
//
//   Update ocb->kbodata according to t and lhs while checking if var
//   occurs in t.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool mfyvwbc(OCB_p ocb, Term_p t, DerefType deref_t, Term_p var, bool lhs)
{
   int i;
   bool res = false, tmp;

   t = TermDeref(t, &deref_t); 
   local_vb_update(ocb, t, lhs);
   
   if(var->f_code == t->f_code)
   {
      res = true;
   }
   else
   {
      for(i=0; i<t->arity; i++)
      {         
         tmp = mfyvwbc(ocb, t->args[i], deref_t, var, lhs);
         res |= tmp;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: mfyvwb()
//
//   Update ocb->kbodata according to t and lhs.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void mfyvwb(OCB_p ocb, Term_p t, DerefType deref_t, bool lhs)
{
   int i;

   t = TermDeref(t, &deref_t); 
   local_vb_update(ocb, t, lhs);

   /* Note that for variables, arity == 0, so this is skipped without
      separate test */
   for(i=0; i<t->arity; i++)
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
   int i;
   
   assert(s->arity == t->arity);
   assert(s->f_code == t->f_code);

   for(i=0; i<s->arity; i++)
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
// Side Effects    : (only in ocb->kbodata)
//
/----------------------------------------------------------------------*/

static CompareResult kbo6cmp(OCB_p ocb, Term_p s, Term_p t,
                             DerefType deref_s, DerefType deref_t)
{
   CompareResult res, lex, g_or_n, l_or_n, tmp;
   bool          ctn;

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
         ctn = mfyvwbc(ocb, t, deref_t, s, false);
         inc_vb(ocb, s);
         res = ctn?to_lesser:to_uncomparable;
      }
   }
   else if(TermIsVar(t))
   { /* s, Y */
      ctn = mfyvwbc(ocb, s, deref_s, t, true);
      dec_vb(ocb, t);
      res = ctn?to_greater:to_uncomparable;      
   }
   else
   { /* s, t */
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
      g_or_n = ocb->kbobalance->neg_bal?to_uncomparable:to_greater;
      l_or_n = ocb->kbobalance->pos_bal?to_uncomparable:to_lesser;
      if(ocb->kbobalance->wb>0)
      {
         res = g_or_n;
      }
      else if(ocb->kbobalance->wb<0)
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
   KBOLinReset(ocb->kbobalance);
   
   return  kbo6cmp(ocb, s, t, deref_s, deref_t);
}



/*-----------------------------------------------------------------------
//
// Function: KBO6Greater(ocb, s, t)
//
//   Checks whether the term s is greater than the term t in the
//   Knuth-Bendix Ordering (KBO), i.e. returns
//
//                       true      if s >KBO t,
//      		 false     otherwise.
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
   KBOLinReset(ocb->kbobalance);
   if(kbo6cmp(ocb, s, t, deref_s, deref_t) == to_greater)
   {
      return true;
   }
   return false;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
