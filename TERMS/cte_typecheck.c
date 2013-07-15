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
#include "cte_termfunc.h"

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
// Function: make_quantifier_type
//  builds the "type" of a quantifier
//   
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
Type_p make_quantifier_type(TypeTable_p table)
{
   SortType ret, args[2];
   Type_p res;

   ret = STBool;
   args[0] = table->sort_table->default_type;
   args[1] = STBool;

   res = TypeNewFunction(table, ret, 2, args);
   return res;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TypeCheckConsistent
//  recursively checks that the subterms of this term have a sort
//  that is consistent with the given signature.
//   
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
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
// Function: TypeInferReturnSort
//  Infers the sort of this term from the signature. If a function symbol
//  is not declared, a default type will be used ($i everywhere).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
SortType TypeInferReturnSort(Sig_p sig, Term_p term)
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
         if (SigIsPredicate(sig, term->f_code)
             || SigQueryProp(sig, term->f_code, FPFOFOp))
         {
            return STBool;
         }
         else if (SigQueryProp(sig, term->f_code, FPIsInteger))
         {
            return STInteger;
         }
         else if (SigQueryProp(sig, term->f_code, FPIsRational))
         {
            return STRational;
         }
         else if (SigQueryProp(sig, term->f_code, FPIsFloat))
         {
            return STReal;
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


/*-----------------------------------------------------------------------
//
// Function: TypeInfer
//  infer the type of this term's function symbol (if it is not a
//  variable), and declare it in the signature if needed.
//  If the type is already known, checks the type and return true iff
//  the type is consistent with the signature.
//   
//
// Global Variables: -
//
// Side Effects    : Updates signature if the type is inferred, modifies the
//                   term's sort
//
/----------------------------------------------------------------------*/

// TODO: in case of equality, use the other side if it has a type

bool TypeInfer(Sig_p sig, Term_p term)
{
   Type_p type;
   SortType sort, *args;
   int i;
   bool res = true;

   if(!TermIsVar(term))
   {
      /* Ad-hoc polymorphism for equality: check that both sides have
       * the same type */
      if(term->f_code == SigGetEqnCode(sig, true))
      {
         res = SortEqual(term->args[0]->sort, term->args[1]->sort);
         term->sort = STBool;
      }
      /* Type inference for quantifiers */
      else if (term->f_code == sig->qex_code || term->f_code == sig->qall_code)
      {
         res = SortEqual(term->args[1]->sort, STBool);
         term->sort = STBool;
      }
      else if (SigQueryProp(sig, term->f_code, FPFOFOp))
      {
         for(i=0; res && i < term->arity; ++i)
         {
            res = res && SortEqual(term->args[i]->sort, STBool);
         }
         term->sort = STBool;
      }
      else
      {
         /* Regular type inference */
         type = SigGetType(sig, term->f_code);

         if(!type)
         {
            /* must infer the type */
            sort = TypeInferReturnSort(sig, term);
            term->sort = sort;

            if(term->arity)
            {
               args = TypeArgumentAlloc(term->arity);
               for(i=0; i < term->arity; ++i)
               {
                   args[i] = term->args[i]->sort;
               }

               type = TypeNewFunction(sig->type_table, sort, term->arity, args);
               TypeArgumentFree(args, term->arity);
            }
            else
            {
               type = TypeNewConstant(sig->type_table, sort);
            }

            SigDeclareType(sig, term->f_code, type);
         }
         else
         {
            term->sort = type->domain_sort;

            res = res && (type->arity == term->arity);

            for(i=0; res && i < term->arity; ++i)
            {
               if(!SortEqual(term->args[i]->sort, type->arguments[i]))
               {
                  res = false;
               }
            }
         }
      }
   }

   if(!res && Verbose>=3)
   {
      fprintf(stderr, "type error in ");
      TermPrint(stderr, term, sig, DEREF_NEVER);
      fprintf(stderr, "\n");
   }

   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TypeInferRec
//  infer the type of this term and its subterms, or checks types
//  if the type is already known, recursively. Returns true
//  if the types are correct or have been inferred.
//   
//
// Global Variables: -
//
// Side Effects    : same as TypeInfer
//
/----------------------------------------------------------------------*/
bool TypeInferRec(Sig_p sig, Term_p term)
{
   int i;
   bool res = true;

   if(!TermIsVar(term))
   {
      for(i=0; res && i < term->arity; ++i)
      {
         res = TypeInferRec(sig, term->args[i]);
      }

      if(res)
      {
         res = TypeInfer(sig, term);
      }
   }

   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


