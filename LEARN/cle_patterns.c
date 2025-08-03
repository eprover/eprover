/*-----------------------------------------------------------------------

  File  : cle_patterns.c

  Author: Stephan Schulz

  Contents

  Copyright 1998, 1999, 2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created:  Wed Apr 14 22:58:47 MET DST 1999

  -----------------------------------------------------------------------*/

#include "cle_patterns.h"



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
// Function: get_new_fun_symbol()
//
//   Return a new norm-id for a given arity.
//
// Global Variables: -
//
// Side Effects    : Changes used_idents;
//
/----------------------------------------------------------------------*/

static FunCode get_new_fun_symbol(PatternSubst_p subst, int arity)
{
   long    base;
   IntOrP* where;

   where = PDArrayElementRef(subst->used_idents, arity);
   where->i_val++;
   base = where->i_val;

   assert(base<=NORM_SYMBOL_LIMIT && "Too many function symbols ???");

   return PatternNormCode(base, arity);
}

/*-----------------------------------------------------------------------
//
// Function: pat_symb_comp_val()
//
//   Return the norm id assigned to f_code, or the alpha-rank if
//   symbol is self-bound.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode pat_symb_comp_val(PatternSubst_p subst, FunCode f_code)
{
   assert(f_code);

   if(f_code > 0)
   {
      FunCode res = PDArrayElementInt(subst->fun_subst, f_code);

      if(res == f_code)
      {
         assert(res<=subst->sig->f_count);
         res = SigGetAlphaRank(subst->sig,f_code);
      }
      return res;
   }
   else if(VarFCodeIsAltCode(f_code))
   {
      return f_code;
   }
   return PDArrayElementInt(subst->var_subst, -f_code);
}


/*-----------------------------------------------------------------------
//
// Function: pat_symbol_compare()
//
//   Compare two function symbols as follows:
//
//   Originally:
//
//   If either symbol is unbound but should be bound, return
//   to_uncomparable. Otherwise, compare symbols
//   numerically (if truly bound) or by their alpha-ranks (if bound to
//   themselves). Fresh variables are not expected to be bound.
//
//   However: During pattern generation, an unbound symbol can only be
//   bound to a larger symbol, as new pattern symbols are given out in
//   ascending order -> We can always consider a bound symbol to be
//   smaller than an unbound one! (Note that symbols compared here
//   always have the same arity as well, otherwise the structure-based
//   ordering captures it!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static CompareResult pat_symbol_compare(PatternSubst_p subst1, FunCode
                                        f1, PatternSubst_p subst2,
                                        FunCode f2)
{
   if(PatSymbolIsBound(subst1, f1) && PatSymbolIsBound(subst2, f2))
   {
      FunCode cmp = pat_symb_comp_val(subst1, f1) -
         pat_symb_comp_val(subst2, f2);

      return Q_TO_PART(cmp);
   }
   else if(PatSymbolIsBound(subst1, f1))
   {
      return to_lesser;
   }
   else if(PatSymbolIsBound(subst1, f2))
   {
      return to_greater;
   }
   return to_uncomparable;
}


/*-----------------------------------------------------------------------
//
// Function: generate_print_rep()
//
//   Given a norm-id, generate the print-representation into *id.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void generate_print_rep(FunCode f, char *id)
{
   assert(PatIdIsNormId(f));
   assert(id);

   sprintf(id, "f%ld_%ld",
           PatternIdGetArity(f),
           PatternIdGetIdent(f));
}


/*-----------------------------------------------------------------------
//
// Function: pat_term_size_compare()
//
//   Compare two terms with (a variant of) the lexicograpic extension
//   to the ordering induced by the default weights. If two symbols of
//   different arities are encountered, the one with the higher arity
//   is always bigger. This ordering is used as the base for pattern
//   comparison and is independend of actual function symbols. Note
//   that terms with more function symbols are smaller in this
//   ordering! Special case: $true is always larger than any other
//   term!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

/* This can now probably be done via TermStructWeightCompare */

static CompareResult pat_term_size_compare(Term_p t1, Term_p t2)
{
   PStack_p      stack;
   CompareResult res   = to_equal;
   long          cmp;
   int           i;

   if((t1->f_code == SIG_TRUE_CODE)&&(t2->f_code == SIG_TRUE_CODE))
   {
      return to_equal;
   }
   if(t1->f_code == SIG_TRUE_CODE)
   {
      return to_greater;
   }
   if(t2->f_code == SIG_TRUE_CODE)
   {
      return to_lesser;
   }

   stack  = PStackAlloc();

   PStackPushP(stack, t1);
   PStackPushP(stack, t2);

   while(!PStackEmpty(stack))
   {
      t2 = PStackPopP(stack);
      t1 = PStackPopP(stack);
      if(t1==t2)
      {
         continue;
      }

      assert(TermStandardWeight(t1) == TermWeight(t1,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
      assert(TermStandardWeight(t2) == TermWeight(t2,DEFAULT_VWEIGHT,DEFAULT_FWEIGHT));
      cmp = TermStandardWeight(t1) - TermStandardWeight(t2);

      if(cmp < 0)
      {
         res = to_greater;
         break;
      }
      else if(cmp > 0)
      {
         res = to_lesser;
         break;
      }
      assert(cmp == 0);

      /* Now either t1 and t2 are variables or neither is! */

      cmp = t1->arity - t2->arity;

      if(cmp < 0)
      {
         res = to_greater;
         break;
      }
      else if(cmp > 0)
      {
         res = to_lesser;
         break;
      }
      assert(cmp == 0);
      for(i=0; i<t1->arity; i++)
      {
         PStackPushP(stack, t1->args[i]);
         PStackPushP(stack, t2->args[i]);
      }
   }
   PStackFree(stack);
   assert((res!=to_uncomparable)&&(res!=to_unknown));
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: initialize_lit_list()
//
//   For all unused literals in list mark all literals as potentially
//   minimal in all possible directions.
//
// Global Variables: -
//
// Side Effects    : Changes properties
//
/----------------------------------------------------------------------*/

static void initialize_lit_list(PatternSubst_p subst, Eqn_p list)
{
   Eqn_p handle;
   CompareResult cmpres;

   for(handle = list; handle; handle = handle->next)
   {
      if(!EqnQueryProp(handle, EPIsUsed))
      {
         EqnDelProp(handle, EPLPatMinimal|EPRPatMinimal);
         cmpres = PatternTermCompare(subst, handle->lterm, subst,
                                     handle->rterm);
         switch(cmpres)
         {
         case to_equal: /* Equal stays equal, i.e. we need to consider
                           only one case! FALLTHROUGH */
         case to_lesser:
               EqnSetProp(handle,EPLPatMinimal);
               break;
         case to_greater:
               EqnSetProp(handle,EPRPatMinimal);
               break;
         case to_uncomparable:
               EqnSetProp(handle,EPLPatMinimal|EPRPatMinimal);
               break;
         case to_unknown:
         default:
               assert(false);
               break;
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: mark_minimal_literals()
//
//   Among all literals in list that do not have EPIsUsed set, mark
//   those literal/direction combinations that are potentially minimal
//   in the semi-complete pattern ordering.
//
// Global Variables: -
//
// Side Effects    : Changes properties in clause.
//
/----------------------------------------------------------------------*/

static void mark_minimal_literals(PatternSubst_p subst, Eqn_p list)
{
   Eqn_p handle, compare;
   CompareResult cmpres;

   initialize_lit_list(subst, list);

   for(handle = list; handle; handle = handle->next)
   {
      if(!EqnIsAnyPropSet(handle, EPLPatMinimal|EPRPatMinimal)||
         EqnQueryProp(handle, EPIsUsed))
      {
         continue;
      }
      for(compare = handle->next; compare; compare = compare->next)
      {
         if(EqnQueryProp(compare, EPIsUsed))
         {
            continue;
         }
         if(EqnQueryProp(handle, EPLPatMinimal))
         {
            if(EqnQueryProp(compare, EPLPatMinimal))
            {
               cmpres = PatternTermPairCompare(subst, handle,
                                               PENormal, subst,
                                               compare, PENormal);
               switch(cmpres)
               {
               case to_equal: /* If two literals are _equal_, this
                                 will not change -> we can arbitrarily
                                 pick exactly one -> FALLTHROUGH */
               case to_lesser:
                     EqnDelProp(compare, EPLPatMinimal);
                     break;
               case to_greater:
                     EqnDelProp(handle, EPLPatMinimal);
                     break;
               default:
                     assert(cmpres!=to_unknown);
                     break;
               }
            }
            if(EqnQueryProp(compare, EPRPatMinimal))
            {
               cmpres = PatternTermPairCompare(subst, handle,
                                               PENormal, subst,
                                               compare, PEReverse);
               switch(cmpres)
               {
               case to_equal: /* If two literals are _equal_, this
                                 will not change -> we can arbitrarily
                                 pick exactly one -> FALLTHROUGH */
               case to_lesser:
                     EqnDelProp(compare, EPRPatMinimal);
                     break;
               case to_greater:
                     EqnDelProp(handle, EPLPatMinimal);
                     break;
               default:
                     assert(cmpres!=to_unknown);
                     break;
               }
            }
         }
         if(EqnQueryProp(handle, EPRPatMinimal))
         {
            if(EqnQueryProp(compare, EPLPatMinimal))
            {
               cmpres = PatternTermPairCompare(subst, handle,
                                               PEReverse, subst,
                                               compare, PENormal);
               switch(cmpres)
               {
               case to_equal: /* If two literals are _equal_, this
                                 will not change -> we can arbitrarily
                                 pick exactly one -> FALLTHROUGH */
               case to_lesser:
                     EqnDelProp(compare, EPLPatMinimal);
                     break;
               case to_greater:
                     EqnDelProp(handle, EPRPatMinimal);
                     break;
               default:
                     assert(cmpres!=to_unknown);
                     break;
               }
            }
            if(EqnQueryProp(compare, EPRPatMinimal))
            {
               cmpres = PatternTermPairCompare(subst, handle,
                                               PEReverse, subst,
                                               compare, PEReverse);
               switch(cmpres)
               {
               case to_equal: /* If two literals are _equal_, this
                                 will not change -> we can arbitrarily
                                 pick exactly one -> FALLTHROUGH */
               case to_lesser:
                     EqnDelProp(compare, EPRPatMinimal);
                     break;
               case to_greater:
                     EqnDelProp(handle, EPRPatMinimal);
                     break;
               default:
                     assert(cmpres!=to_unknown);
                     break;
               }
            }
         }
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: collect_choices()
//
//   For a list of literals collect all possible choices for the next
//   literal to appear in the pattern for the list. Returns number of
//   possibilities, choices are represented by pairs
//   (literal,direction) on the choice stack.
//
// Global Variables: -
//
// Side Effects    : Changes literal properties
//
/----------------------------------------------------------------------*/

static int collect_choices(PatternSubst_p subst, Eqn_p list, PStack_p
                           choices)
{
   int i = 0;
   Eqn_p handle;

   mark_minimal_literals(subst, list);
   for(handle = list; handle; handle = handle->next)
   {
      if(EqnQueryProp(handle, EPIsUsed))
      {
         continue;
      }
      if(EqnQueryProp(handle, EPLPatMinimal))
      {
         PStackPushP(choices, handle);
         PStackPushInt(choices, PENormal);
         i++;
      }
      if(EqnQueryProp(handle, EPRPatMinimal))
      {
         PStackPushP(choices, handle);
         PStackPushInt(choices, PEReverse);
         i++;
      }
   }
   return i;
}

/*-----------------------------------------------------------------------
//
// Function: complete_state()
//
//   Complete a state in the search. A state is described by
//
//   - A list of literals, some of which may be used up already
//     (marked with EPIsUsed)
//   - A stack which contains exactly the used equations (and
//     determines their order in the pattern)
//   - A partial pattern substitution generated from the used literals
//   - A state stack which organizes the search. It contains 3 entries
//     per picked literal:
//     + The pattern-subst pointer before selecting the literal
//     + A stack of choices
//
//  Return true if successful, false if operation is to expensive.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static bool complete_state(PatternSubst_p subst, Eqn_p list,
                           PStack_p order, PStack_p state)
{
   PStack_p choices = PStackAlloc();
   int      choice_nr;
   Eqn_p    picked;
   PatEqnDirection dir;

   choice_nr = collect_choices(subst, list, choices);

   while(choice_nr && (choice_nr <= PATTERN_SEARCH_BRANCHLIMIT))
   {
      assert(PStackGetSP(choices) >= 2);
      dir    = PStackPopInt(choices);
      picked = PStackPopP(choices);

      PStackPushInt(state, PStackGetSP(subst->backtrack));
      PStackPushP(state, choices);
      EqnSetProp(picked, EPIsUsed);
      PatternTermPairCompute(subst, picked, dir);
      PStackPushP(order, picked);
      PStackPushInt(order, dir);

      choices = PStackAlloc();
      choice_nr = collect_choices(subst, list, choices);
   }
   PStackFree(choices);

   return (choice_nr == 0);
}

/*-----------------------------------------------------------------------
//
// Function: lit_list_rep_pattern()
//
//   Generate the representative pattern for a list of
//   equations. Return number of possibilities tried, or 0 if routine
//   terminates because the cost is estimated as to expensive.
//
// Global Variables: -
//
// Side Effects    : Memory management, flips properties.
//
/----------------------------------------------------------------------*/

static long lit_list_rep_pattern(Eqn_p list, PatternSubst_p* subst,
                                 PStack_p *order)
{
   PStack_p        best_order = NULL;
   PatternSubst_p  best_subst = NULL;
   PStack_p        state = PStackAlloc();
   PStack_p        choices;
   Eqn_p           picked;
   PatEqnDirection dir;
   PStackPointer   old_sp;
   long            count = 1;
   bool            affordable;

   EqnListDelProp(list, EPIsUsed);

   affordable = complete_state(*subst, list, *order, state);
   best_subst = PatternSubstCopy(*subst);
   best_order = PStackCopy(*order);

   while((!PStackEmpty(state)) && affordable)
   {
      assert(PStackGetSP(state) >= 2);
      assert(PStackGetSP(*order) >= 2);
      PStackDiscardTop(*order); /* Direction */
      picked = PStackPopP(*order);
      EqnDelProp(picked, EPIsUsed);
      old_sp = PStackBelowTopInt(state);
      PatternSubstBacktrack(*subst, old_sp);
      choices = PStackTopP(state);
      if(!PStackEmpty(choices))
      {
         count++;
         assert(PStackGetSP(choices) >= 2);
         dir    = PStackPopInt(choices);
         picked = PStackPopP(choices);

         EqnSetProp(picked, EPIsUsed);
         PatternTermPairCompute(*subst, picked, dir);
         PStackPushP(*order, picked);
         PStackPushInt(*order, dir);
         affordable = complete_state(*subst, list, *order, state);

         if(affordable)
         {
            if(PatternLitListCompare(*subst, *order,
                                     best_subst, best_order)
               == to_lesser)
            {
               PatternSubstFree(best_subst);
               PStackFree(best_order);
               best_subst = PatternSubstCopy(*subst);
               best_order = PStackCopy(*order);
            }
         }
      }
      else
      {
         PStackFree(choices);
         PStackDiscardTop(state); /* Choices */
         PStackDiscardTop(state); /* Old SP */
      }
   }
   while(!PStackEmpty(state))
   {
      choices = PStackPopP(state);/* Choices */
      PStackFree(choices);
      PStackDiscardTop(state);      /* Old SP */
   }
   PStackFree(state);
   assert(!affordable || PStackEmpty(*order));
   PStackFree(*order);
   PatternSubstFree(*subst);
   *order = best_order;
   *subst = best_subst;

   return affordable?count:0;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: PatternSubstAlloc()
//
//   Allocate an empty initialized pattern-substitution cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PatternSubst_p PatternSubstAlloc(Sig_p sig)
{
   PatternSubst_p handle = PatternSubstCellAlloc();

   handle->used_idents = PDIntArrayAlloc(DEFAULT_SIGNATURE_SIZE,
                                         DEFAULT_SIGNATURE_SIZE);
   handle->fun_subst   = PDIntArrayAlloc(DEFAULT_SIGNATURE_SIZE,
                                         DEFAULT_SIGNATURE_SIZE);
   handle->used_vars   = NORM_VAR_INIT;
   handle->var_subst   = PDIntArrayAlloc(DEFAULT_VARBANK_SIZE,
                                         DEFAULT_VARBANK_SIZE);
   handle->backtrack   = PStackAlloc();
   handle->sig         = sig;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: PatternDefaultSubstAlloc()
//
//   Allocate an empty initialized pattern-substitution cell where all
//   special function symbols are bound to themselves.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PatternSubst_p PatternDefaultSubstAlloc(Sig_p sig)
{
   PatternSubst_p handle = PatternSubstAlloc(sig);
   FunCode i;

   for(i=1; i<=sig->f_count; i++)
   {
      if(SigIsSpecial(sig, i))
      {
         PDArrayAssignInt(handle->fun_subst, i,i);
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PatternSubstFree()
//
//   Free the memory taken by a pattern-subst cell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void PatternSubstFree(PatternSubst_p junk)
{
   PDArrayFree(junk->used_idents);
   PDArrayFree(junk->fun_subst);
   PDArrayFree(junk->var_subst);
   PStackFree(junk->backtrack);

   PatternSubstCellFree(junk);
}



/*-----------------------------------------------------------------------
//
// Function: PatSubstExtend()
//
//   Extend the pattern subst to substitute symbol (if not already
//   done). Return true if the subst has been extended.
//
// Global Variables: -
//
// Side Effects    : Changes used_idents, extends subst
//
/----------------------------------------------------------------------*/

bool PatSubstExtend(PatternSubst_p subst, FunCode symbol, int arity)
{
   long res;
   bool result = false;

   if(symbol > 0)
   {
      res = PDArrayElementInt(subst->fun_subst, symbol);
      if(!res)
      {
         res = get_new_fun_symbol(subst, arity);
         PDArrayAssignInt(subst->fun_subst, symbol, res);
         PStackPushInt(subst->backtrack, symbol);
         result = true;
      }
   }
   else
   {
      assert(symbol < 0);
      if(!VarFCodeIsAltCode(symbol))
      {
         res = PDArrayElementInt(subst->var_subst, -symbol);
         if(!res)
         {
            res = --subst->used_vars;
            PDArrayAssignInt(subst->var_subst, -symbol, res);
            PStackPushInt(subst->backtrack, symbol);
            result = true;
         }
      }
   }
   return result;
}



/*-----------------------------------------------------------------------
//
// Function: PatternSubstCopy()
//
//   Copy a pattern-substitution.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

PatternSubst_p PatternSubstCopy(PatternSubst_p subst)
{
   PatternSubst_p handle = PatternSubstCellAlloc();

   handle->used_idents = PDArrayCopy(subst->used_idents);
   handle->fun_subst   = PDArrayCopy(subst->fun_subst);
   handle->used_vars   = subst->used_vars;
   handle->var_subst   = PDArrayCopy(subst->var_subst);
   handle->backtrack   = PStackCopy(subst->backtrack);
   handle->sig         = subst->sig;

   return handle;

}

/*-----------------------------------------------------------------------
//
// Function: PatSymbValue()
//
//   Return the norm id assigned to f_code.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

FunCode PatSymbValue(PatternSubst_p subst, FunCode f_code)
{
   assert(f_code);

   if(f_code > 0)
   {
      if(SigIsSpecial(subst->sig, f_code))
      {
         return f_code;
      }
      return PDArrayElementInt(subst->fun_subst, f_code);
   }
   else if(VarFCodeIsAltCode(f_code))
   {
      return f_code;
   }
   return PDArrayElementInt(subst->var_subst, -f_code);
}

/*-----------------------------------------------------------------------
//
// Function: PatSymbolIsBound()
//
//   Return true is f_code is either bound to a symbol or should not
//   be bound at all.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PatSymbolIsBound(PatternSubst_p subst, FunCode f_code)
{
   return (PatSymbValue(subst, f_code) != 0);
}


/*-----------------------------------------------------------------------
//
// Function: PatternSubstBacktrack()
//
//   Backtrack a pattern-subst to a given state. Return true if the
//   state differs from the current one.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool PatternSubstBacktrack(PatternSubst_p subst, PStackPointer
                           old_state)
{
   bool    res = false;
   FunCode symbol, rep_symbol;

   assert(subst);
   assert(old_state <= PStackGetSP(subst->backtrack));

   while(PStackGetSP(subst->backtrack) > old_state)
   {
      symbol = PStackPopInt(subst->backtrack);

      if(symbol < 0)
      {
         rep_symbol = PDArrayElementInt(subst->var_subst, -symbol);
         assert(subst->used_vars == rep_symbol);
         subst->used_vars++;
         PDArrayAssignInt(subst->var_subst, -symbol, 0);
      }
      else
      {
         int arity, count;
         IntOrP *tmp;

         rep_symbol = PDArrayElementInt(subst->fun_subst, symbol);
         arity = PatternIdGetArity(rep_symbol);
         count = PatternIdGetIdent(rep_symbol);

         tmp = PDArrayElementRef(subst->used_idents, arity);
         UNUSED(count); assert(tmp->i_val == count);
         tmp->i_val--;
         PDArrayAssignInt(subst->fun_subst, symbol, 0);
      }

      res = true;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PatternTermCompute()
//
//   Extend subst to make term into a pattern. Return true if a new
//   renaming has been added.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

bool PatternTermCompute(PatternSubst_p subst, Term_p term)
{
   int i;
   bool res, tmp;

   assert(term);
   assert(subst);

   res = PatSubstExtend(subst, term->f_code, term->arity);

   for(i=0; i< term->arity; i++)
   {
      assert(!TermIsFreeVar(term));
      assert(term->args[i]);
      tmp = PatternTermCompute(subst, term->args[i]);
      res = res||tmp;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PatternTermCompare()
//
//   Compare two term patterns.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

CompareResult PatternTermCompare(PatternSubst_p subst1, Term_p t1,
                                 PatternSubst_p subst2, Term_p t2)
{
   CompareResult res;

   res = pat_term_size_compare(t1, t2);
   assert((res!=to_uncomparable)&&(res!=to_unknown));

   if(res != to_equal)
   {
      return res;
   }
   else
   {
      PStack_p stack = PStackAlloc();
      int i;

      PStackPushP(stack, t1);
      PStackPushP(stack, t2);

      while(!PStackEmpty(stack))
      {
         t2 = PStackPopP(stack);
         t1 = PStackPopP(stack);

         assert(t1->arity == t2->arity);

         res = pat_symbol_compare(subst1, t1->f_code, subst2,
                                  t2->f_code);
         if(res != to_equal)
         {
            break;
         }
         for(i=0; i<t1->arity; i++)
         {
            PStackPushP(stack, t1->args[i]);
            PStackPushP(stack, t2->args[i]);
         }
      }
      PStackFree(stack);
      return res;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PatternTermPairCompute()
//
//   Compute a pattern subst for a given equation.
//
// Global Variables: -
//
// Side Effects    : Changes subst.
//
/----------------------------------------------------------------------*/
bool PatternTermPairCompute(PatternSubst_p subst, Eqn_p eqn,
                            PatEqnDirection direction)
{
   bool res, tmp;

   if(!direction)
   {
      res = PatternTermCompute(subst, eqn->lterm);
      tmp = PatternTermCompute(subst, eqn->rterm);
      res = res || tmp;
   }
   else
   {
      res = PatternTermCompute(subst, eqn->rterm);
      tmp = PatternTermCompute(subst, eqn->lterm);
      res = res || tmp;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PatternTermPairCompare()
v//
//   Compare two equation patterns (described by pattern-subst, eqn,
//   direction).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

CompareResult PatternTermPairCompare(PatternSubst_p subst1, Eqn_p
                                     eqn1, PatEqnDirection dir1,
                                     PatternSubst_p subst2, Eqn_p
                                     eqn2, PatEqnDirection dir2)
{
   Term_p l1,l2,r1,r2;
   CompareResult res;
   long cmpres;

   cmpres = EqnStandardWeight(eqn2) - EqnStandardWeight(eqn1);

   if(cmpres)
   {
      return Q_TO_PART(cmpres);
   }

   l1 = PatEqnLTerm(eqn1,dir1);
   r1 = PatEqnRTerm(eqn1,dir1);
   l2 = PatEqnLTerm(eqn2,dir2);
   r2 = PatEqnRTerm(eqn2,dir2);

   res = pat_term_size_compare(l1, l2);
   if(res!=to_equal)
   {
      return res;
   }
   res = pat_term_size_compare(r1, r2);
   if(res!=to_equal)
   {
      return res;
   }

   if(EqnIsPositive(eqn1))
   {
      if(EqnIsNegative(eqn2))
      {
         return to_greater;
      }
   }
   else
   {
      if(EqnIsPositive(eqn2))
      {
         return to_lesser;
      }
   }

   res = PatternTermCompare(subst1, l1, subst2, l2);
   if(res != to_equal)
   {
      return res;
   }
   res = PatternTermCompare(subst1, r1, subst2, r2);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PatternLitListCompute()
//
//   Compute a pattern-subst for the list (well, stack) of oriented
//   literals.
//
// Global Variables: -
//
// Side Effects    : Changes subst
//
/----------------------------------------------------------------------*/

bool PatternLitListCompute(PatternSubst_p subst, PStack_p listrep)
{
   bool  res = false, tmp;
   Eqn_p eqn;
   PatEqnDirection dir;
   PStackPointer i;

   for(i=0; i<PStackGetSP(listrep); i+=2)
   {
      assert(i+1 < PStackGetSP(listrep));
      eqn = PStackElementP(listrep, i);
      dir = PStackElementInt(listrep, i+1);

      tmp = PatternTermPairCompute(subst, eqn, dir);
      res = res || tmp;
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PatternLitListCompare()
//
//   Compare two patterns for clauses, each represented by a pattern
//   substitution and a stack of (oriented) literals. This does not
//   correspond exactly to the definition in my thesis, but agrees
//   with it on comparisons of different patterns for the same clause
//   (while being easier and more efficient in the general case, where
//   it does not matter).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

CompareResult PatternLitListCompare(PatternSubst_p subst1, PStack_p
                                    listrep1, PatternSubst_p subst2,
                                    PStack_p listrep2)
{
   long cmpres;
   PStackPointer i;
   Eqn_p eqn1, eqn2;
   int dir1, dir2;
   CompareResult res = to_equal;

   cmpres = PStackGetSP(listrep1)-PStackGetSP(listrep2);
   if(cmpres)
   {
      return Q_TO_PART(cmpres);
   }
   for(i=0; i < PStackGetSP(listrep1); i+=2)
   {
      assert(i+1 < PStackGetSP(listrep1));
      eqn1 = PStackElementP(listrep1, i);
      dir1 = PStackElementInt(listrep1, i+1);
      eqn2 = PStackElementP(listrep2, i);
      dir2 = PStackElementInt(listrep2, i+1);

      res = PatternTermPairCompare(subst1, eqn1, dir1, subst2, eqn2,
                                   dir2);
      if(res != to_equal)
      {
         break;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PatternClauseCompute()
//
//   Compute the representative pattern for a clause and return it
//   (via the reference variables). Returns number of substitutions
//   tried, or 0 if the internal resource estimator canceled the
//   computation.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may flip some literal flags.
//
/----------------------------------------------------------------------*/

long PatternClauseCompute(Clause_p clause, PatternSubst_p* subst,
                          PStack_p *listrep)
{
   long res;

   res = lit_list_rep_pattern(clause->literals, subst, listrep);

   /* printf(COMCHAR" %d literals, %ld tries\n", ClauseLiteralNumber(clause),
      res);
      if(res > 500)
      {
      printf(COMCHAR" Large clause: ");
      ClausePrint(stdout, clause, true);
      printf("\n");
      } */
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PatternTermPrint()
//
//   Print the pattern-term to out. Supports only standard E syntax.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PatternTermPrint(FILE* out, PatternSubst_p subst, Term_p term,
                      Sig_p sig)
{
   FunCode id;
   int  i;
   char new_id[27];

   id = PatSymbValue(subst, term->f_code);
   if(TermIsFreeVar(term))
   {
      if(!id)
      {
         fprintf(out, "X%ld", -term->f_code);
      }
      else
      {
         fprintf(out, "Xn%ld", -(id-NORM_VAR_INIT));
      }
   }
   else
   {
      if(id >= NORM_SYMBOL_LIMIT)
      {
         generate_print_rep(id, new_id);
         fputs(new_id, out);
      }
      else
      {
         fputs(SigFindName(sig, term->f_code), out);
      }
      if(term->arity)
      {
         fprintf(out, "(");
         PatternTermPrint(out, subst, term->args[0], sig);

         for(i=1; i<term->arity; i++)
         {
            fprintf(out, ",");
            PatternTermPrint(out, subst, term->args[i], sig);
         }
         fprintf(out, ")");
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: PatternEqnPrint()
//
//   Print a pattern equation in the most reasonable form.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void PatternEqnPrint(FILE* out, PatternSubst_p subst, Eqn_p eqn,
                     PatEqnDirection direction)
{
   if(EqnIsEquLit(eqn))
   {
      if(!direction)
      {
         PatternTermPrint(out, subst, eqn->lterm, eqn->bank->sig);
         fputs(EqnIsPositive(eqn)?"=":"!=", out);
         PatternTermPrint(out, subst, eqn->rterm, eqn->bank->sig);
      }
      else
      {
         PatternTermPrint(out, subst, eqn->rterm, eqn->bank->sig);
         fputs(EqnIsPositive(eqn)?"=":"!=", out);
         PatternTermPrint(out, subst, eqn->lterm, eqn->bank->sig);
      }
   }
   else
   {
      if(!EqnIsPositive(eqn))
      {
         fputc('~', out);
      }
      PatternTermPrint(out, subst, eqn->lterm, eqn->bank->sig);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PatternClausePrint()
//
//   Print the clause pattern represented by listrep and
//   subst and print it as a LOP list of literals. This format is
//   primarily for machine use. Regularity and compactness are more
//   important than beauty.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void PatternClausePrint(FILE* out, PatternSubst_p subst, PStack_p
                        listrep)
{
   Eqn_p eqn;
   PatEqnDirection dir;
   PStackPointer i;
   char* prefix = "";

   for(i=0; i<PStackGetSP(listrep); i+=2)
   {
      assert(i+1 < PStackGetSP(listrep));
      eqn = PStackElementP(listrep, i);
      dir = PStackElementInt(listrep, i+1);
      fputs(prefix, out);
      PatternEqnPrint(out, subst, eqn, dir);
      prefix = ";";
   }
   fputs(" <- .", out);
}

/*-----------------------------------------------------------------------
//
// Function: DebugPatternClauseToStack()
//
//   Generate the straightforward stack representation of clause
//   (probably useful only for debugging and testing). The calling
//   function has to return the stack!
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

PStack_p DebugPatternClauseToStack(Clause_p clause)
{
   PStack_p res = PStackAlloc();
   Eqn_p    handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      PStackPushP(res, handle);
      PStackPushInt(res, PENormal);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PatternTranslateSig()
//
//   Create a copy of (uninstantiated) term in new_sig and new_vars,
//   inserting all
//   necessary idents and print-representations of norm-idents into
//   new_sig. Norm-substituted variables are mapped to their unnormed
//   counterparts.
//
// Global Variables: -
//
// Side Effects    : Changes new_sig,  changes
//
/----------------------------------------------------------------------*/

Term_p PatternTranslateSig(Term_p term, PatternSubst_p subst, Sig_p
                           old_sig, Sig_p new_sig, VarBank_p new_vars)
{
   char     new_id[27];
   char*    id;
   PStack_p stack = PStackAlloc();
   Term_p   t, new;
   FunCode  f_code;
   int      i;

   /* Deal with variables...this is messy ;-) */
   PStackPushP(stack, term);
   while(!PStackEmpty(stack))
   {
      t = PStackPopP(stack);
      if(TermIsFreeVar(t))
      {
         f_code = PatSymbValue(subst, t->f_code);
         if(f_code)
         {
            t->binding = VarBankVarAssertAlloc(new_vars, f_code-NORM_VAR_INIT, t->type);
         }
      }
      else
      {
         for(i=0; i<t->arity; i++)
         {
            PStackPushP(stack, t->args[i]);
         }
      }
   }

   new = TermCopy(term, new_vars, NULL,  DEREF_ONCE);
   /* Reset variables */
   PStackPushP(stack, term);
   while(!PStackEmpty(stack))
   {
      t = PStackPopP(stack);
      if(TermIsFreeVar(t))
      {
         t->binding = NULL;
      }
      else
      {
         for(i=0; i<t->arity; i++)
         {
            PStackPushP(stack, t->args[i]);
         }
      }
   }

   PStackPushP(stack, new);

   while(!PStackEmpty(stack))
   {
      t = PStackPopP(stack);
      if(TermIsFreeVar(t))
      {
         continue;
      }
      f_code = PatSymbValue(subst, t->f_code);
      if(PatIdIsNormId(f_code))
      {
         generate_print_rep(f_code, new_id);
         t->f_code = SigInsertId(new_sig, new_id, t->arity, false);
         assert(t->f_code);
      }
      else
      {
         id = SigFindName(old_sig, t->f_code);
         assert(id);
         t->f_code = SigInsertId(new_sig, id, t->arity, false);
         assert(t->f_code);
      }
      for(i=0; i<t->arity; i++)
      {
         PStackPushP(stack, t->args[i]);
      }
   }
   PStackFree(stack);
   return new;
}

/*-----------------------------------------------------------------------
//
// Function: PatternSubstGetOriginalSymbol()
//
//   Given a symbol f, return the original FunCode. Return 0 if f_code
//   does not match any known symbol.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

FunCode PatternSubstGetOriginalSymbol(PatternSubst_p subst, FunCode f)
{
   long i;

   if(f > 0)
   {
      for(i=0; i<subst->fun_subst->size; i++)
      {
         if(PDArrayElementInt(subst->fun_subst, i)==f)
         {
            return i;
         }
      }
      return 0;
   }
   else
   {
      if(VarFCodeIsAltCode(f))
      {
         return f;
      }
      else
      {
         for(i=0; i<subst->fun_subst->size; i++)
         {
            if(PDArrayElementInt(subst->var_subst, i)==f)
            {
               return i;
            }
         }
         return 0;
      }
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
