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
// Function: infer_return_sort
//
//   infer the return sort of the given function symbol, given the signature.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
SortType infer_return_sort(Sig_p sig, FunCode f_code)
{
   SortType res;

   if(SigQueryProp(sig, f_code, FPIsInteger) &&
      (sig->distinct_props & FPIsInteger))
   {
      res = STInteger;
   }
   else if (SigQueryProp(sig, f_code, FPIsRational) &&
            (sig->distinct_props & FPIsRational))
   {
      res = STRational;
   }
   else if (SigQueryProp(sig, f_code, FPIsFloat) &&
            (sig->distinct_props & FPIsFloat))
   {
      res = STReal;
   }
   else
   {
      res = SigDefaultSort(sig);
   }

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
   Type_p type;

   PStack_p stack = PStackAlloc();
   PStackPushP(stack, term);

   while (!PStackEmpty(stack))
   {
      term = PStackPopP(stack);

      if (!TermIsVar(term))
      {
         /* check: same arity, same return sort, sort of arguments (pairwise)*/
         if(!SigIsPolymorphic(sig, term->f_code))
         {
            type = SigGetType(sig, term->f_code);

            assert(type);

            if((term->arity != type->arity)
               || term->sort != type->domain_sort)
            {
               res = false;
               break;
            }

            /* Check subterms recursively */
            for (int i=0; i < type->arity; i++)
            {
               PStackPushP(stack, term->args[i]);
               if(term->args[i]->sort != type->args[i])
               {
                  res = false;
                  break;
               }
            }
         }
      }
   }

   PStackFree(stack);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TypeInferSort
//
//   Infer the sort of this term. It can either use the type of the
//   function symbol, if already known, or guess a type and add it
//   to the signature otherwise. By default terms are supposed not to
//   be atoms, unless the parser decides that they must be boolean.
//
// Global Variables: -
//
// Side Effects    : Modifies term and signature. May exit on type error.
//
/----------------------------------------------------------------------*/
void TypeInferSort(Sig_p sig, Term_p term)
{
   Type_p type;
   SortType sort, *args;
   int i;

   if(TermIsVar(term))
   {
      if(term->sort == STNoSort)
      {
         term->sort = SigDefaultSort(sig);
      }
   }
   else
   {
      type = SigGetType(sig, term->f_code);

      /* Use type */
      if(type)
      {
         if(term->arity != type->arity)
         {
            fprintf(stderr, "# arity mismatch for ");
            TermPrint(stderr, term, sig, DEREF_NEVER);
            fprintf(stderr, " and type ");
            TypePrintTSTP(stderr, sig->type_table, type);
            fprintf(stderr, "\n");
            Error("Type error", SYNTAX_ERROR);
         }

         assert(term->arity == type->arity);
         for(i=0; SigIsFixedType(sig, term->f_code) && i < term->arity; i++)
         {
            if(term->args[i]->sort != type->args[i])
            {
               fprintf(stderr, "# Type mismatch in argument #%d of ", i+1);
               TermPrint(stderr, term, sig, DEREF_NEVER);
               fprintf(stderr, ": expected ");
               SortPrintTSTP(stderr, sig->sort_table, type->args[i]);
               fprintf(stderr, " but got ");
               SortPrintTSTP(stderr, sig->sort_table, term->args[i]->sort);
               fprintf(stderr, "\n");
               Error("Type error", SYNTAX_ERROR);
            }
         }

         term->sort = type->domain_sort;
      }
      else
      {
         /* Infer type */
         sort = infer_return_sort(sig, term->f_code);
         args = TypeArgumentAlloc(term->arity);
         for(i=0; i < term->arity; i++)
         {
            args[i] = term->args[i]->sort;
         }

         type = TypeNewFunction(sig->type_table, sort, term->arity, args);
         TypeArgumentFree(args, term->arity);

         /* Declare the inferred type */
         SigDeclareType(sig, term->f_code, type);
         term->sort = sort;
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: TypeDeclareIsPredicate
//
//   declare that the term has a role of predicate (occurs as a boolean atom)
//
// Global Variables: -
//
// Side Effects    : Modifies sig, modifies term's sort
//
/----------------------------------------------------------------------*/
void TypeDeclareIsPredicate(Sig_p sig, Term_p term)
{
   assert(!TermIsVar(term));

   SigDeclareIsPredicate(sig, term->f_code);
   term->sort = STBool;
}


/*-----------------------------------------------------------------------
//
// Function: TypeDeclareIsNotPredicate
//
//   Declare that this term is not a boolean atom, because it ocurs in
//   an equation or is a subterm of another term.
//
// Global Variables: -
//
// Side Effects    : Modifies signature, update term's sort
//
/----------------------------------------------------------------------*/
void TypeDeclareIsNotPredicate(Sig_p sig, Term_p term)
{
   if(!TermIsVar(term))
   {
      TypeInferSort(sig, term);
      SigDeclareIsFunction(sig, term->f_code);
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


