/*-----------------------------------------------------------------------

File  : cte_typecheck.c

Author: Simon Cruanes

Contents

  Type checking and inference for Simple types
 

  Copyright 2011 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1>   Mon Jul  8 17:15:05 CEST 2013
      New

-----------------------------------------------------------------------*/

#include "cte_typecheck.h"


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

bool TypeCheckConsistent(Sig_p sig, Term_p term)
{
   bool res = true;
   PStack_p stack;
   Term_p subterm;
   int i;
   SortType sort;
   Type_p type;

   stack = PStackAlloc();
   PStackPushP(stack, term);

   while (res && !PStackEmpty(stack))
   {
      term = PStackPopP(stack);

      if (!TermIsVar(term))
      {
         sort = term->sort;
         type = SigGetType(sig, term->f_code);

         /* check: same arity, same return sort, sort of arguments (pairwise)*/
         res = res && (term->arity == type->arity);
         res = res && SortEqual(sort, type->domain_sort);
         for (i=0; res && i < type->arity; ++i)
         {
            subterm = term->args[i];
            res = res && SortEqual(subterm->sort, type->arguments[i]);
            
            /* Check subterms themselves */
            PStackPushP(stack, subterm);
         }
      }
   }

   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TypeInfer
//  Infers the sort of this term from the signature. If a function symbol
//  is not declared, a default type will be used ($i everywhere).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
SortType TypeInfer(Sig_p sig, Term_p term)
{
   Type_p type;

   if (TermIsVar(term))
   {
      if (term->sort == STNoSort)
      {
         return SigDefaultSort(sig);
      }
      else
      {
         return term->sort;
      }
   }
   else
   {
      type = SigGetType(sig, term->f_code); 
      if (type == sig->type_table->no_type)
      {
         /* Take the predicate flag into account */
         if (SigIsPredicate(sig, term->f_code))
         {
            return STBool;
         }
         else
         {
            return SigDefaultSort(sig);
         }
      }
      else
      {
         return type->domain_sort;
      }
   }
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


