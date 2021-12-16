/*-----------------------------------------------------------------------

File  : cle_flatannoterms.c

Author: Stephan Schulz

Contents

  Functions for dealing with flat annotated terms.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Aug  9 12:59:39 MET DST 1999
    New

-----------------------------------------------------------------------*/

#include "cle_flatannoterms.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoTermAlloc()
//
//   Return a flatly annotated term.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FlatAnnoTerm_p FlatAnnoTermAlloc(Term_p term, double eval, double
             eval_weight, long sources)
{
   FlatAnnoTerm_p handle = FlatAnnoTermCellAlloc();

   handle->term        = term;
   handle->eval        = eval;
   handle->eval_weight = eval_weight;
   handle->sources     = sources;
   handle->next        = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoTermPrint()
//
//   Print a flatly annotated term "t:eval." (mostly for debugging, I
//   suppose)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FlatAnnoTermPrint(FILE* out, FlatAnnoTerm_p term, Sig_p sig)
{
   TermPrint(out, term->term, sig, DEREF_NEVER);
   fprintf(out, " : %f. /* EvalWeight: %f, Id: %ld */", term->eval,
      term->eval_weight, term->term->entry_no);
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetAlloc()
//
//   Allocate a flatly annotated term set.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

FlatAnnoSet_p FlatAnnoSetAlloc(void)
{
   FlatAnnoSet_p handle = FlatAnnoSetCellAlloc();

   handle->set   = NULL;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetFree()
//
//   Free a set of flatly annotated terms.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void FlatAnnoSetFree(FlatAnnoSet_p junk)
{
   NumTree_p handle;
   PStack_p stack;

   stack = NumTreeTraverseInit(junk->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      FlatAnnoTermFree(handle->val1.p_val);
   }
   NumTreeTraverseExit(stack);

   NumTreeFree(junk->set);
   FlatAnnoSetCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetPrint()
//
//   Print a set o flatly annotated terms (mostly for debugging, I
//   suppose)
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void FlatAnnoSetPrint(FILE* out, FlatAnnoSet_p set, Sig_p sig)
{
   NumTree_p cell;
   PStack_p stack = NumTreeTraverseInit(set->set);

   while((cell = NumTreeTraverseNext(stack)))
   {
      FlatAnnoTermPrint(out, cell->val1.p_val, sig);
      fputc('\n', out);
   }
   NumTreeTraverseExit(stack);
}

/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetAddTerm()
//
//   Add a flatly annotated term to a set. If the term already exists,
//   merge the annotations and free the original annotateted term.
//
// Global Variables: -
//
// Side Effects    : Changes set
//
/----------------------------------------------------------------------*/

bool FlatAnnoSetAddTerm(FlatAnnoSet_p set, FlatAnnoTerm_p term)
{
   NumTree_p exists;
   FlatAnnoTerm_p existing_term;
   IntOrP    tmp;
   bool res = true;

   exists = NumTreeFind(&(set->set), term->term->entry_no);
   if(exists)
   {
      res = false;
      existing_term = exists->val1.p_val;
      existing_term->eval = (term->eval * term->eval_weight +
              existing_term->eval *
              existing_term->eval_weight)/
    (term->eval_weight + existing_term->eval_weight);
      existing_term->eval_weight += term->eval_weight;
      existing_term->sources += term->sources;
      FlatAnnoTermFree(term);
   }
   else
   {
      tmp.p_val = term;
      NumTreeStore(&(set->set), term->term->entry_no, tmp, tmp);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetTranslate()
//
//  Given a set of annotated terms with exactly one annotation per
//  term, generate a corrsponding flatly annotated term set.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes flatset
//
/----------------------------------------------------------------------*/

long FlatAnnoSetTranslate(FlatAnnoSet_p flatset, AnnoSet_p set, double
           weights[])
{
   NumTree_p      handle;
   PStack_p       stack;
   FlatAnnoTerm_p term;
   AnnoTerm_p     old;
   long           res = 0;
   bool           check;

   stack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      old = handle->val1.p_val;
      assert(old->annotation);
      assert(!old->annotation->lson);
      assert(!old->annotation->lson);
      term = FlatAnnoTermAlloc(old->term,
                AnnotationEval(old->annotation,
                     weights),
                (double)AnnotationCount(old->annotation)
                /*
             /(double)TermWeight(old->term, 1, 1)
                */
                ,
                AnnotationCount(old->annotation));

      check = FlatAnnoSetAddTerm(flatset, term);
      UNUSED(check); assert(check);
      res++;
   }
   NumTreeTraverseExit(stack);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetSize()
//
//   Return the number of terms in a flatanno-set (counting sources).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FlatAnnoSetSize(FlatAnnoSet_p fset)
{
   NumTree_p handle;
   PStack_p stack;
   long res = 0;
   FlatAnnoTerm_p term;

   stack = NumTreeTraverseInit(fset->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      term = handle->val1.p_val;
      res+=term->sources;
   }
   NumTreeTraverseExit(stack);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoTermFlatten()
//
//   Generate a fresh annoterm for each subterm of term (inheriting
//   the original annotation with modiefied weight) and insert it into
//   set. Returns number of terms created.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

long FlatAnnoTermFlatten(FlatAnnoSet_p set, FlatAnnoTerm_p term)
{
   long res = 0;
   PStack_p stack = PStackAlloc();
   Term_p         t;
   FlatAnnoTerm_p handle;
   int i;

   PStackPushP(stack, term->term);
   while(!PStackEmpty(stack))
   {
      t = PStackPopP(stack);
      handle = FlatAnnoTermAlloc(t, term->eval, term->eval_weight,
             term->sources);
      FlatAnnoSetAddTerm(set, handle);
      res++;
      for(i=0; i<t->arity; i++)
      {
    assert(t->args);
    PStackPushP(stack, t->args[i]);
      }
   }
   assert(PStackEmpty(stack));
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetFlatten()
//
//   For all terms in to_flatten and all subterms, insert them into
//   set. Return number of terms created.
//
// Global Variables: -
//
// Side Effects    : By FlatAnnoTermFlatten()
//
/----------------------------------------------------------------------*/

long FlatAnnoSetFlatten(FlatAnnoSet_p set, FlatAnnoSet_p to_flatten)
{
   NumTree_p handle;
   PStack_p stack;
   long res = 0;

   stack = NumTreeTraverseInit(to_flatten->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      res+=FlatAnnoTermFlatten(set, handle->val1.p_val);
   }
   NumTreeTraverseExit(stack);

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetEvalAverage()
//
//   Return the average of all evaluation for terms in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double FlatAnnoSetEvalAverage(FlatAnnoSet_p set)
{
   NumTree_p handle;
   PStack_p stack;
   long sources = 0;
   double res = 0;
   FlatAnnoTerm_p term;

   if(!set->set)
   {
      return 0;
   }

   stack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      term = handle->val1.p_val;
      res+=term->eval;
      sources+=term->sources;
   }
   NumTreeTraverseExit(stack);

   return res/(double)sources;
}


/*-----------------------------------------------------------------------
//
// Function: FlatAnnoSetEvalWeightedAverage()
//
//   Return the weighted average of all evaluation for terms in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double FlatAnnoSetEvalWeightedAverage(FlatAnnoSet_p set)
{
   NumTree_p handle;
   PStack_p stack;
   double weight = 0;
   double res = 0;
   FlatAnnoTerm_p term;

   if(!set->set)
   {
      return 0;
   }

   stack = NumTreeTraverseInit(set->set);

   while((handle = NumTreeTraverseNext(stack)))
   {
      term = handle->val1.p_val;
      res+=term->eval_weight*term->eval;
      weight+=term->eval_weight;
   }
   NumTreeTraverseExit(stack);

   return res/weight;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


