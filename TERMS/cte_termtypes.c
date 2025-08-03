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
#include "cte_termbanks.h"
#include "cte_lambda.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#ifdef ENABLE_LFHO
/*-----------------------------------------------------------------------
//
// Function: register_new_cache()
//
//   Stores the new (binding cache, bound to) pair for applied variable.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static __inline__ void register_new_cache(Term_p app_var, Term_p bound_to)
{
   assert(TermIsAppliedFreeVar(app_var));
   assert(app_var->args[0]->binding);

   app_var->binding = app_var->args[0]->binding;
   TermSetCache(app_var, bound_to);

   TermCellSetProp(TermGetCache(app_var), TPIsDerefedAppVar);
}

/*-----------------------------------------------------------------------
//
// Function: insert_deref()
//
//   Makes sure that the dereferenced applied variable is shared.
//   Due to term replacing it might be the case that some arguments
//   are shared and some are not.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p insert_deref(Term_p deref_cache, TB_p bank)
{
   for(int i=0; i<deref_cache->arity; i++)
   {
      if(!TermIsFreeVar(deref_cache->args[i]) && !TermIsShared(deref_cache->args[i]))
      {
         deref_cache->args[i] = TBInsertIgnoreVar(bank, deref_cache->args[i], DEREF_NEVER);
      }
   }

   return TBTermTopInsert(bank, deref_cache);
}


/*-----------------------------------------------------------------------
//
// Function: clear_stale_cache()
//
//   Clears the cache if it is not up to date. Assumes that cache
//   is stale (see BINDING_FRESH).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void clear_stale_cache(Term_p app_var)
{
   assert(TermIsAppliedFreeVar(app_var));
   assert(!BINDING_FRESH(app_var));

   TermSetCache(app_var, NULL);
   app_var->binding = NULL;
}


/*-----------------------------------------------------------------------
//
// Function: applied_var_deref()
//
//   Expands applied variable to a proper term. For example, if X is bound
//   to f a, term X b would get expanded to f a b.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

__inline__ Term_p applied_var_deref(Term_p orig)
{
   assert(TermIsAppliedFreeVar(orig));
   assert(orig->arity > 1);
   assert(orig->args[0]->binding || TermGetCache(orig));

   Term_p res;

   if(BINDING_FRESH(orig))
   {
      assert(TermCellQueryProp(TermGetCache(orig), TPIsDerefedAppVar));
      res = TermGetCache(orig);
   }
   else
   {
      clear_stale_cache(orig);

      if(orig->args[0]->binding)
      {
         if(TermIsAnyVar(orig->args[0]->binding) || TermIsLambda(orig->args[0]->binding))
         {
            res = TermTopAlloc(orig->f_code, orig->arity);
            res->properties = orig->properties & (TPPredPos);
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

            res->type = orig->type; // derefing keeps the types
            res->properties = bound->properties & (TPPredPos);

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

         register_new_cache(orig, (res = insert_deref(res, TermGetBank(orig))));
      }
      else
      {
         res = orig;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: TermFindUnownedSubterm()
//
//   Check if term has at least one subterm without term->owner_bank
//   set. At the moment only useful for debugging...
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Term_p TermFindUnownedSubterm(Term_p term)
{
   Term_p res;

   if(!term->owner_bank)
   {
      return term;
   }
   for(int i=0; i<term->arity; i++)
   {
      if((res = TermFindUnownedSubterm(term->args[i])))
      {
         return res;
      }
   }
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: DBGTermCheckUnownedSubtermReal()
//
//   Check for unowned subterms, if found, print them with a marker
//   and a location string.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void DBGTermCheckUnownedSubtermReal(FILE* out, Term_p t, char* location)
{
   if(TermFindUnownedSubterm(t))
   {
      fprintf(out, COMCHAR" UnknownSubterm(%s): ", location);
      TermPrint(stdout, t, t->owner_bank->sig, DEREF_NEVER);
      fprintf(stdout, "\n");
   }
}

#endif


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
   assert(junk);
   TermCellFree(junk, junk->arity);
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
   if(!TermIsAnyVar(junk))
   {
      assert(!TermCellQueryProp(junk, TPIsShared));
      if(junk->arity)
      {
         int i;

         for(i=0; i<junk->arity; i++)
         {
            TermFree(junk->args[i]);
         }
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
   PStackPointer arity = PStackGetSP(variables), i;
   Term_p handle = NULL;

   Type_p *type_args;
   Type_p type;

   if(!ret_type)
   {
      ret_type = SigDefaultSort(sig);
   }

   // declare type
   if(arity)
   {
      handle = TermDefaultCellArityAlloc(arity);

      type_args = TypeArgArrayAlloc(arity+1);
      for(i=0; i<arity; i++)
      {
         handle->args[i] = PStackElementP(variables, i);
         type_args[i] = handle->args[i]->type;
         assert(type_args[i]);
      }
      type_args[arity] = ret_type;

      type = AllocArrowType(arity+1, type_args);
      Type_p flattened = FlattenType(type);
      if(flattened != type)
      {
         TypeFree(type);
      }
      type = flattened;
   }
   else
   {
      handle = TermDefaultCellAlloc();
      type = FlattenType(ret_type);
   }

   type = TypeBankInsertTypeShared(sig->type_bank, type);

   if(!TypeIsPredicate(type))
   {
      handle->f_code = SigGetNewSkolemCode(sig, PStackGetSP(variables));
   }
   else
   {
      handle->f_code = SigGetNewPredicateCode(sig, PStackGetSP(variables));
   }

   SigDeclareType(sig, handle->f_code, type);
   handle->type = ret_type;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: TermSetProp()
//
//   Set the properties in all term cells belonging to term.
//   NB: The function is never called with deref once -- no changes
//       to DEREF_ONCE
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermSetProp(Term_p term, DerefType deref, TermProperties prop)
{
   assert(deref != DEREF_ONCE);
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
//   NB: Deref not changed -- function never used.
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
//   NB: Derefs not changed -- function never called with DEREF_ONCE.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermVerifyProp(Term_p term, DerefType deref, TermProperties prop,
                    TermProperties expected)
{
   assert(deref != DEREF_ONCE);
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
//    NB: Derefs not changed -- function never called with DEREF_ONCE
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermDelProp(Term_p term, DerefType deref, TermProperties prop)
{
   assert(deref != DEREF_ONCE);
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
//   NB: Derefs not changed -- function never called with DEREF_ONCE
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermVarSetProp(Term_p term, DerefType deref, TermProperties prop)
{
   assert(deref != DEREF_ONCE);
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsFreeVar(term))
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
      /* printf(COMCHAR"Fcode: %ld  Sort: %d\n", term->f_code, term->sort); */
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
//   NB: Derefs never changed -- function not called with DEREF_ONCE
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermVarSearchProp(Term_p term, DerefType deref, TermProperties prop)
{
   assert(deref != DEREF_ONCE);
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
      if(TermIsFreeVar(term) && TermCellQueryProp(term, prop))
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
//   NB: Derefs not changed -- function not called with DEREF_ONCE
//
// Global Variables: -
//
// Side Effects    : Changes properties (even in shared terms! Beware!)
//
/----------------------------------------------------------------------*/

void TermVarDelProp(Term_p term, DerefType deref, TermProperties prop)
{
   assert(deref != DEREF_ONCE);
   PStack_p stack = PStackAlloc();
   int i;

   PStackPushP(stack, term);
   PStackPushInt(stack, deref);

   while(!PStackEmpty(stack))
   {
      deref = PStackPopInt(stack);
      term  = PStackPopP(stack);
      term  = TermDeref(term, &deref);
      if(TermIsFreeVar(term))
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

/*-----------------------------------------------------------------------
//
// Function: TermIsPrefix()
//
//   Checks if candidate is a prefix of term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool TermIsPrefix(Term_p cand, Term_p term)
{
   assert(problemType == PROBLEM_HO);
   bool res = false;
   int  i;
   if(cand)
   {
      /* cand can be null if it was binding field of non-bound var,
         which is common use case for this function  */

      if(TermIsAnyVar(cand))
      {
         return TermIsAnyVar(term) ? cand == term :
                  (TermIsPhonyApp(term) ? cand == term->args[0] : false);
      }

      if(cand->arity <= term->arity && cand->f_code == term->f_code)
      {
         for(i=0; i<cand->arity; i++)
         {
            if(cand->args[i] != term->args[i])
            {
               break;
            }
         }
         res = i == cand->arity;
      }
   }
   return res;
}


#ifdef ENABLE_LFHO
/*-----------------------------------------------------------------------
//
// Function: MakeRewrittenTerm()
//
//   Rewrite the prefix of orig using new, leaving remaining_orig
//   arguments of orig intact.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

__inline__ Term_p MakeRewrittenTerm(Term_p orig, Term_p new,
                                    int remaining_orig,
                                    struct tbcell* bank)
{
   if(remaining_orig)
   {
      assert(problemType == PROBLEM_HO);
      Term_p new_term;
      if(TermIsFreeVar(new))
      {
         new_term = TermTopAlloc(SIG_PHONY_APP_CODE, remaining_orig+1);
         new_term->args[0] = new;
      }
      else
      {
         new_term = TermTopAlloc(new->f_code, new->arity + remaining_orig);
      }

      new_term->type = orig->type; // no type inference after this step
      new_term->properties = orig->properties & (TPPredPos);

      for(int i=0; i < new->arity; i++)
      {
         new_term->args[i] = new->args[i];
      }
      for(int i=orig->arity - remaining_orig, j=TermIsFreeVar(new) ? 1 : 0;
          i < orig->arity;
          i++, j++)
      {
         new_term->args[j + new->arity] = orig->args[i];
      }

      TermSetBank(new_term, bank);

      return LambdaNormalizeDB(bank, new_term);
   }
   else
   {
      TermSetBank(new, bank);
      return LambdaNormalizeDB(bank, new); // If no args are remaining -- the situation is the same as in FO case
   }
}
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
