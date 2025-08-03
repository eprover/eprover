/*-----------------------------------------------------------------------

  File  : cte_acterms.c

  Author: Stephan Schulz

  Contents

  Funktions for dealing with AC-normalized terms

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov 22 00:31:03 CET 2000

  -----------------------------------------------------------------------*/

#include "cte_acterms.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: acterm_uniq_compare()
//
//   Compare two AC-Terms first lexicographically and then by
//   top-level-pointer. Two copies of the same term compare as
//   different here.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int acterm_uniq_compare(const void* term1, const void* term2)
{
   const ACTerm_p t1 = (const ACTerm_p) term1;
   const ACTerm_p t2 = (const ACTerm_p) term2;
   int res = ACTermCompare(t1, t2);

   if(!res)
   {
      res = PCmp(t1,t2);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ac_collect_args()
//
//   Collect all subterms of the AC-Symbol f in the orderd tree
//   anchored at *root.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static void ac_collect_args(PTree_p* root, Sig_p sig, FunCode f,
                            Term_p term)
{

   if(term->f_code != f)
   {
      PTree_p res;

      ACTerm_p tmp = ACTermNormalize(sig, term);

      res = PTreeObjStore(root, tmp, acterm_uniq_compare);
      UNUSED(res); assert(!res);
   }
   else
   {
      int i;

      for(i=0; i < term->arity; i++)
      {
         ac_collect_args(root, sig, f, term->args[i]);
      }
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ACTermAlloc()
//
//   Allocate an initialized AC-Term cell
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ACTerm_p ACTermAlloc(FunCode f)
{
   ACTerm_p handle = ACTermCellAlloc();

   handle->f_code = f;
   if(f >0)
   {
      handle->args = PDArrayAlloc(3,2);
   }
   else
   {
      handle->args = NULL;
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ACTermFree()
//
//   Free an AC-Term.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ACTermFree(ACTerm_p term)
{
   assert(term);

   if(term->f_code > 0)
   {
      int i;
      ACTerm_p t;

      for(i=0; (t=PDArrayElementP(term->args, i)); i++)
      {
         ACTermFree(t);
      }
      PDArrayFree(term->args);
   }
   else
   {
      assert(!term->args);
   }
   ACTermCellFree(term);
}

/*-----------------------------------------------------------------------
//
// Function: ACTermCompare()
//
//   Compare two AC terms lexicograpically.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int ACTermCompare(ACTerm_p t1, ACTerm_p t2)
{
   assert(t1 && t2);

   int res =
      (t1->f_code == SIG_DB_LAMBDA_CODE || t2->f_code == SIG_DB_LAMBDA_CODE) ?
      -1 : CMP(t1->f_code, t2->f_code);

   if(res == 0 && t1->f_code > 0)
   {
      int i;
      ACTerm_p arg1,arg2;

      assert(t1->args&&t2->args);

      for(i=0; !res; i++)
      {
         arg1 = PDArrayElementP(t1->args, i);
         arg2 = PDArrayElementP(t2->args, i);
         if(!arg1 && !arg2)
         {
            break;
         }
         else if(!arg1)
         {
            res = -1;
            break;
         }
         else if(!arg2)
         {
            res = 1;
            break;
         }
         res = ACTermCompare(arg1, arg2);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ACTermNormalize()
//
//   Transform a CLIB term into an AC term in AC-normalform.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ACTerm_p ACTermNormalize(Sig_p sig, Term_p term)
{
   ACTerm_p handle = ACTermAlloc(term->f_code);

   if(!TermIsAnyVar(term) && !TermIsLambda(term) && (term->arity != 0))
   {
      int i;

      if(SigQueryFuncProp(sig, term->f_code, FPIsAC))
      {
         PTree_p args = NULL, cell;
         PStack_p stack;

         ac_collect_args(&args, sig, term->f_code, term);

         i=0;
         stack = PTreeTraverseInit(args);
         while((cell = PTreeTraverseNext(stack)))
         {
            PDArrayAssignP(handle->args,i++, cell->key);
         }
         PTreeTraverseExit(stack);
         PTreeFree(args);
      }
      // in LFHOL symbol must not be fully applied
      else if(SigQueryFuncProp(sig, term->f_code, FPCommutative) && term->arity == 2)
      {
         ACTerm_p t1, t2, tmp;

         t1 = ACTermNormalize(sig,term->args[0]);
         t2 = ACTermNormalize(sig,term->args[1]);

         if(ACTermCompare(t1, t2) > 0)
         {
            tmp = t1;
            t1 = t2;
            t2 = tmp;
         }
         PDArrayAssignP(handle->args,0,t1);
         PDArrayAssignP(handle->args,1,t2);
      }
      else
      {
         for(i=0; i<term->arity; i++)
         {
            PDArrayAssignP(handle->args,i,
                           ACTermNormalize(sig,term->args[i]));
         }
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ACTermPrint()
//
//   Print an AC-Normalized term in flat form.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void ACTermPrint(FILE* out, ACTerm_p term, Sig_p sig)
{
   if(term->f_code < 0)
   {
      VarPrint(out, term->f_code);
   }
   else
   {
      int i;
      ACTerm_p arg;

      assert(term->args);
      fputs(SigFindName(sig, term->f_code),out);
      arg = PDArrayElementP(term->args, 0);

      if(arg)
      {
         putc('(', out);
         ACTermPrint(out, arg, sig);
         for(i=1; (arg = PDArrayElementP(term->args, i)); i++)
         {
            putc(',', out);
            ACTermPrint(out, arg, sig);
         }
         putc(')', out);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermACEqual()
//
//   Return true if the two terms are equal modulo AC as described in
//   the signature.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

bool TermACEqual(Sig_p sig, Term_p t1, Term_p t2)
{
   bool res = true;

   if(TermStandardWeight(t1)!=TermStandardWeight(t2)
      || TermIsPhonyApp(t1) || TermIsPhonyApp(t2))
   {
      res = false;
   }
   else
   {
      ACTerm_p nt1, nt2;

      nt1 = ACTermNormalize(sig, t1);
      nt2 = ACTermNormalize(sig, t2);

      /* printf("\n"COMCHAR" T-1: ");
         TermPrint(stdout, t1, sig, DEREF_NEVER);
         printf("\n"COMCHAR" T-2: ");
         TermPrint(stdout, t2, sig, DEREF_NEVER);

         printf("\n"COMCHAR" AC1: ");
         ACTermPrint(stdout, nt1, sig);
         printf("\n"COMCHAR" AC2: ");
         ACTermPrint(stdout, nt2, sig);*/
      if(ACTermCompare(nt1, nt2)!=0)
      {
         res = false;
      }
      /* printf("\n"COMCHAR" RES: %d\n", res); */
      ACTermFree(nt2);
      ACTermFree(nt1);
   }
   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
