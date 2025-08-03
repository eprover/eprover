/*-----------------------------------------------------------------------

  File  : cte_typecheck.c

  Author: Simon Cruanes, Petar Vucmirovic, Stephan Schulz

  Contents

  Type checking and inference for Simple types

  Copyright 2011-2024 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Mon Jul  8 17:15:05 CEST 2013

  -----------------------------------------------------------------------*/

#include "cte_typecheck.h"
#include "cte_termfunc.h"
#include <cte_typebanks.h>

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/
extern bool app_encode;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: term_determine_type()
//
//   Given number of arguments and type, return the type of the term
//   resulting from consuming the number of arguments. Returned type is
//   shared.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Type_p term_determine_type(Term_p term, Type_p type, TypeBank_p bank)
{
   int term_arity = ARG_NUM(term);
   if(type->arity-1 == term_arity)
   {
      return type->args[term_arity];
   }
   else if(type->arity-1 < term_arity)
   {
      return NULL;
   }
   else
   {
      int start = term_arity;
      Type_p* args = TypeArgArrayAlloc(type->arity - start);
      for(int i=0; i<type->arity-start; i++)
      {
         args[i] = type->args[i+start];
      }
      return TypeBankInsertTypeShared(bank,
                                      AllocArrowType(type->arity - start, args));
   }
}

/*-----------------------------------------------------------------------
//
// Function: infer_return_sort
//
//   Infer the default return sort of the given function symbol, given
//   the signature. This is probably out of date. (StS)
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


Type_p infer_return_sort(Sig_p sig, FunCode f_code)
{
   Type_p res;

   if(SigQueryProp(sig, f_code, FPIsInteger) &&
      (sig->distinct_props & FPIsInteger))
   {
      res = sig->type_bank->integer_type;
   }
   else if(SigQueryProp(sig, f_code, FPIsRational) &&
            (sig->distinct_props & FPIsRational))
   {
      res = sig->type_bank->rational_type;
   }
   else if(SigQueryProp(sig, f_code, FPIsFloat) &&
            (sig->distinct_props & FPIsFloat))
   {
      res = sig->type_bank->real_type;
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

      if (!TermIsFreeVar(term))
      {
         /* check: same arity, same return sort, sort of arguments (pairwise)*/
         if(!SigIsPolymorphic(sig, term->f_code))
         {
            type = SigGetType(sig, term->f_code);

            assert(type);

            if(TypeIsArrow(type))
            {
               if((term->arity != type->arity-1)
                  || term->type != type->args[type->arity-1])
               {
                  res = false;
                  break;
               }
            }
            else
            {
               if(term->arity != 0 || term->type != type)
               {
                  // other kind of type constructor
                  res = false;
                  break;
               }
            }
            /* Check subterms recursively */
            for(int i=0; i < type->arity; i++)
            {
               PStackPushP(stack, term->args[i]);
               if(term->args[i]->type != type->args[i])
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
// Function: TypeInferSort()
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

#define TI_ERROR(msg) do{ \
   if(in)                                       \
   {\
      AktTokenError(in, msg, false);            \
   }\
   else\
   {\
      Error(msg, SYNTAX_ERROR);\
   }}while(false)


void TypeInferSort(Sig_p sig, Term_p term, Scanner_p in)
{
   Type_p type = NULL;
   Type_p sort, *args;
   int i;

   if(TermIsFreeVar(term))
   {
      if(!term->type)
      {
         term->type = SigDefaultSort(sig);
      }
   }
   else
   {
      if(TermIsPhonyApp(term))
      {
         type = term->args[0]->type;
      }
      else if(TermIsLambda(term))
      {
         assert(term->f_code != SIG_DB_LAMBDA_CODE || TermIsDBVar(term->args[0]));
         assert(term->f_code != SIG_NAMED_LAMBDA_CODE || TermIsFreeVar(term->args[0]));
         // types have to be inferred only during parsing
         assert(term->arity == 2);
         // type of lambda is 'type of variable' -> 'type of body'
         term->type =
            TypeBankInsertTypeShared(sig->type_bank,
                                     ArrowTypeFlattened(&(term->args[0]->type), 1,
                                     term->args[1]->type));
      }
      else if(term->f_code == sig->eqn_code || term->f_code == sig->neqn_code)
      {
         if(term->arity == 0)
         {
            TI_ERROR("Equality must have at least one argument");
         }
         Type_p arg_type = term->args[0]->type;
         Type_p eq_type_args[3] = {arg_type, arg_type, sig->type_bank->bool_type};
         type = TypeBankInsertTypeShared(sig->type_bank,
                                         AllocArrowTypeCopyArgs(3, eq_type_args));
      }
      else if(term->f_code == sig->qex_code || term->f_code == sig->qall_code)
      {
         if(term->arity == 0)
         {
            TI_ERROR("Quantifiers must have at least one argument");
         }
         if(TermIsFreeVar(term->args[0]))
         {
            Type_p arg_type = term->args[0]->type;
            Type_p quant_type_args[3] =
               {arg_type,
               sig->type_bank->bool_type,
               sig->type_bank->bool_type};
            type = TypeBankInsertTypeShared(sig->type_bank,
                                          AllocArrowTypeCopyArgs(3, quant_type_args));
         }
         else
         {
            if(!TypeIsArrow(term->args[0]->type) || !TypeIsPredicate(term->args[0]->type))
            {
               TI_ERROR("Wrong encoding of quantifier arguments");
            }
            Type_p quant_type_args[2] =
               {term->args[0]->type, sig->type_bank->bool_type};
            type = TypeBankInsertTypeShared(sig->type_bank,
                                          AllocArrowTypeCopyArgs(2, quant_type_args));
         }
      }
      else
      {
         type = SigGetType(sig, term->f_code);
      }

      /* Use type */
      if(type)
      {
         if(TypeIsArrow(type))
         {
            if(problemType == PROBLEM_FO && !app_encode
               && term->arity != type->arity-1)
            {
               fprintf(stderr, "Arity mismatch for ");
               TermPrint(stderr, term, sig, DEREF_NEVER);
               fprintf(stderr, " and type ");
               TypePrintTSTP(stderr, sig->type_bank, type);
               fprintf(stderr, "\n");
               TI_ERROR("Type error");
            }

            if(!TermIsPhonyApp(term))
            {
               for(i=0; SigIsFixedType(sig, term->f_code) && i < term->arity; i++)
               {
                  if(term->args[i]->type != type->args[i])
                  {
                     fprintf(stderr, COMCHAR" Type mismatch in argument #%d of ", i+1);
#ifdef ENABLE_LFHO
                     TermPrintDbg(stderr, term, sig, DEREF_NEVER);
#else
                     TermPrintFO(stderr, term, sig, DEREF_NEVER);
#endif
                     fprintf(stderr, ": expected ");
                     TypePrintTSTP(stderr, sig->type_bank, type->args[i]);
                     fprintf(stderr, " but got ");
                     TypePrintTSTP(stderr, sig->type_bank, term->args[i]->type);
                     fprintf(stderr, "\n");
                     assert(false);
                     TI_ERROR("Type error");
                  }
               }
            }
            else
            {
               // probably can be unified with harder to understand code!
               for(i=1; i < term->arity; i++)
               {
                  assert(term->arity-1 < type->arity);

                  if(term->args[i]->type != type->args[i-1])
                  {
                     fprintf(stderr, COMCHAR" Type mismatch in argument #%d of ", i+1);
                     TermPrint(stderr, term, sig, DEREF_NEVER);
                     fprintf(stderr, ": expected ");
                     TypePrintTSTP(stderr, sig->type_bank, type->args[i-1]);
                     fprintf(stderr, " but got ");
                     TypePrintTSTP(stderr, sig->type_bank, term->args[i]->type);
                     fprintf(stderr, "\n");
                     assert(false);
                     TI_ERROR("Type error");
                  }
               }
            }
            term->type = term_determine_type(term, type, sig->type_bank);
            if(term->type==NULL)
            {
               fprintf(stderr, COMCHAR" too many arguments supplied for %s\n",
                       SigFindName(sig, term->f_code));
               assert(false);
               TI_ERROR("Type error");
            }
         }
         else
         {
            if(term->arity != 0)
            {
               fprintf(stderr, COMCHAR" Type mismatch for ");
               TermPrint(stderr, term, sig, DEREF_NEVER);
               fprintf(stderr, " and type ");
               TypePrintTSTP(stderr, sig->type_bank, type);
               fprintf(stderr, "\n");
               assert(false);
               TI_ERROR("Type error");
            }
            else
            {
               term->type = type;
            }
         }
      }
      else if(!TermIsLambda(term))
      {
         /* Infer type */
         sort = infer_return_sort(sig, term->f_code);
         args = term->arity ? TypeArgArrayAlloc(term->arity+1) : NULL;
         for(i=0; i < term->arity; i++)
         {
            args[i] = term->args[i]->type;
         }
         if(term->arity)
         {
            args[term->arity] = sort;
         }

         type = term->arity ?
            TypeBankInsertTypeShared(sig->type_bank,
                                     AllocArrowType(term->arity+1, args))
            : sort;

         /* Declare the inferred type */
         if(!SigIsPolymorphic(sig, term->f_code))
         {
            SigDeclareType(sig, term->f_code, type);
         }
         term->type = sort;
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: TypeDeclareIsPredicate()
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
   assert(!TermIsFreeVar(term));

   SigDeclareIsPredicate(sig, term->f_code);
   term->type = sig->type_bank->bool_type;
}


/*-----------------------------------------------------------------------
//
// Function: TypeDeclareIsNotPredicate()
//
//   Declare that this term is not a boolean atom, because it ocurs in
//   an equation or is a subterm of another term.
//
// Global Variables: -
//
// Side Effects    : Modifies signature, update term's sort
//
/----------------------------------------------------------------------*/

void TypeDeclareIsNotPredicate(Sig_p sig, Term_p term, Scanner_p in)
{
   if(!TermIsAnyVar(term) && term->f_code > sig->internal_symbols)
   {
      TypeInferSort(sig, term, in);
      SigDeclareIsFunction(sig, term->f_code);
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
