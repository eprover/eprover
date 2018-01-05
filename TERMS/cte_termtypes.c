/*-----------------------------------------------------------------------

  File  : cte_termtypes.c

  Author: Stephan Schulz

  Contents

  Declarations for the basic term type and primitive functions, mainly
  on single term cells. This module mostly provides only
  infrastructure for higher level modules.


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Feb 24 02:17:11 MET 1998 - Split  from cte_terms.c

-----------------------------------------------------------------------*/

#include "cte_termtypes.h"

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
// Function: TermDeref()
//
//   Dereference a term. deref* tells us how many derefences to do
//   at most, it will be decremented for each dereferenciation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: TermTopFree()
//
//   Return term cell and arg array (if it exists).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TermTopFree(Term_p junk)
{
   if(junk->arity)
   {
      assert(junk->args);
      TermArgArrayFree(junk->args, junk->arity);
   }
   else
   {
      assert(!junk->args);
   }
   TermCellFree(junk);
}

/*-----------------------------------------------------------------------
//
// Function:  TermFree()
//
//   Return the memory taken by an (unshared) term. Does not free the
//   variable cells, which belong to a VarBank.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void TermFree(Term_p junk)
{
   assert(junk);
   if(!TermIsVar(junk))
   {
      assert(!TermCellQueryProp(junk, TPIsShared));
      if(junk->arity)
      {
         int i;

         assert(junk->args);
         for(i=0; i<junk->arity; i++)
         {
            TermFree(junk->args[i]);
         }
      }
      else
      {
         assert(!junk->args);
      }
      TermTopFree(junk);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermNewSkolemTerm()
//
//   Create a new Skolem term (or renaming atom) with the named
//   variables as arguments.
//
// Global Variables: -
//
// Side Effects    : Memory operations, creates new Skolem function in
//                   sig.
//
/----------------------------------------------------------------------*/

Term_p TermAllocNewSkolem(Sig_p sig, PStack_p variables, Type_p ret_type)
{
   Term_p handle = TermDefaultCellAlloc();
   PStackPointer arity = PStackGetSP(variables), i;
   Type_p *type_args;
   Type_p type;

   if(!ret_type)
   {
      ret_type = SigDefaultSort(sig);
   }

   // declare type
   if(arity)
   {
      handle->arity = arity;
      handle->args = TermArgArrayAlloc(arity);
      type_args = TypeArgArrayAlloc(arity+1);
      for(i=0; i<arity; i++)
      {
         handle->args[i] = PStackElementP(variables, i);
         type_args[i] = handle->args[i]->type;
         assert(type_args[i]);
      }
      type_args[arity] = ret_type;

      type = AllocArrowType(arity+1, type_args);
   }
   else
   {
      type = ret_type;
   }

   type = TypeBankInsertTypeShared(sig->type_bank, FlattenType(type));

   if(!TypeIsPredicate(type))
   {
      handle->f_code = SigGetNewSkolemCode(sig, TypeGetSymbolArity(type));
   }
   else
   {
      handle->f_code = SigGetNewPredicateCode(sig, TypeGetSymbolArity(type));
   }

   SigDeclareFinalType(sig, handle->f_code, type);
   handle->type = ret_type;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermSetProp()
//
//   Set the properties in all term cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermSetProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      TermCellSetProp(term, prop);
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermSearchProp()
//
//   If prop is set in any subterm of term, return true, otherwise
//   false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermSearchProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool res = false;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermCellQueryProp(term, prop))
      {
         res = true;
         break;
      }
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermVerifyProp()
//
//   If prop has the expected value in all subterms of term, return
//   true.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermVerifyProp(Term_p term, DerefType deref, TermProperties prop,
                    TermProperties expected)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool res = true;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermCellGiveProps(term, prop)!=expected)
      {
         res = false;
         break;
      }
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermDelProp()
//
//   Delete the properties in all term cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermDelProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      TermCellDelProp(term, prop);
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermDelPropOpt()
//
//   Delete the properties in all term cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermDelPropOpt(Term_p term, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      term  = PStackPopP(stack);
      TermCellDelProp(term, prop);
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
      }
   }
   PStackFree(stack);
}




/*-----------------------------------------------------------------------
//
// Function: TermVarSetProp()
//
//   Set the properties in all variable cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermVarSetProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsVar(term))
      {
         TermCellSetProp(term, prop);
      }
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermHasInterpretedSymbol()
//
//    Return true if the term has at least one symbol from an
//    interpreted sort (currently the arithmetic sorts,
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool TermHasInterpretedSymbol(Term_p term)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool res = false;

   PStackPushP(stack, term);

   while(!PStackEmpty(stack))
   {
      term  = PStackPopP(stack);
      /* printf("#Fcode: %ld  Sort: %d\n", term->f_code, term->sort); */
      if(SortIsInterpreted(term->type->f_code))
      {
         res = true;
         break;
      }
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
      }
   }
   PStackFree(stack);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: TermVarSearchProp()
//
//   If prop is set in any variable cell in term, return true, otherwise
//   false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermVarSearchProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;
   bool res = false;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsVar(term) && TermCellQueryProp(term, prop))
      {
         res = true;
         break;
      }
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: TermVarDelProp()
//
//   Delete the properties in all variable cells belonging to term.
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermVarDelProp(Term_p term, DerefType deref, TermProperties prop)
{
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsVar(term))
      {
         TermCellDelProp(term, prop);
      }
      for(i=0; i<term->arity; i++)
      {
         PStackPushP(stack, term->args[i]);
         PStackPushInt(stack, deref);
      }
   }
   PStackFree(stack);
}


/*-----------------------------------------------------------------------
//
// Function: TermStackSetProps()
//
//   Set the given properties in all term cells on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermStackSetProps(PStack_p stack, TermProperties prop)
{
   PStackPointer i;
   Term_p term;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      term = PStackElementP(stack, i);
      TermCellSetProp(term, prop);
   }
}


/*-----------------------------------------------------------------------
//
// Function: TermStackDelProps()
//
//   Delete the given properties in all term cells on the stack.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TermStackDelProps(PStack_p stack, TermProperties prop)
{
   PStackPointer i;
   Term_p term;

   for(i=0; i<PStackGetSP(stack); i++)
   {
      term = PStackElementP(stack, i);
      TermCellDelProp(term, prop);
   }
}


bool TermIsPrefix(Term_p needle, Term_p haystack)
{
   assert(ProblemIsHO == PROBLEM_IS_HO);
   bool res = false;
   int  i;
   if (needle)
   {
      /* needle can be null if it was binding field of non-bound var,
         which is common use case for this function 
       */
      if (needle->arity <= haystack->arity && needle->f_code == haystack->f_code) 
      {
         for(i=0; i<needle->arity; i++)
         {
            if (needle->args[i] != haystack->args[i])
            {
               break;
            }
         }

         res = i == needle->arity;
      }
   }
   
   return res;
}

Term_p applied_var_deref(Term_p orig)
{
   assert(TermIsAppliedVar(orig));
   assert(orig->arity > 1);
   assert(orig->args[0]->binding);

   Term_p res;

   if (TermIsVar(orig->args[0]->binding))
   {
      // if deref once , this is OK.
      //assert(orig->args[0] != orig->args[0]->binding);

      res = TermTopAlloc(orig->f_code, orig->arity);
      res->properties = orig->properties;
      res->type = orig->type;
      res->args[0] = orig->args[0]->binding;
      for(int i=1; i<orig->arity; i++)
      {
         res->args[i] = orig->args[i];
      }
   }
   else
   {
      Term_p bound = orig->args[0]->binding;
      int arity = bound->arity + orig->arity-1;

      res = TermTopAlloc(bound->f_code, arity);
      res->args = TermArgArrayAlloc(arity);

      res->type = orig->type; // derefing keeps the types
      res->properties = bound->properties & (TPPredPos | TPIsAppVar);

      assert(!res->binding || res->f_code < 0 /* if bound -> then variable */);

      for(int i=0; i<bound->arity; i++)
      {
         res->args[i] = bound->args[i];
      }

      for(int i=0; i<orig->arity-1; i++)
      {
         res->args[bound->arity + i] = orig->args[i + 1];
      }
   }

   return res;
}

__inline__ Term_p MakeRewrittenTerm(Term_p orig, Term_p new, int remaining_orig)
{  
   if (remaining_orig)
   {
      Term_p new_term;
      if (TermIsVar(new))
      {
         new_term = TermTopAlloc(SIG_APP_VAR_CODE, remaining_orig+1);
         new_term->args[0] = new;
      }
      else
      {
         new_term = TermTopAlloc(new->f_code, new->arity + remaining_orig);
      }

      

      new_term->type = orig->type; // no inference after this step -- speedup.
      new_term->properties = orig->properties & (TPIsAppVar | TPPredPos);

      for(int i=0; i < new->arity; i++)
      {
         new_term->args[i] = new->args[i];
      }
      for(int i=orig->arity - remaining_orig, j=TermIsVar(new) ? 1 : 0; i < orig->arity; i++, j++)
      {
         new_term->args[j + new->arity] = orig->args[i];
      }

      return new_term;
   }
   else
   {
      return new;
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
