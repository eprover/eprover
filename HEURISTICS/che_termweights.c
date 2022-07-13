/*-----------------------------------------------------------------------

  File  : che_termweights.c

  Author: Stephan Schulz, yan

  Contents
 
  Common functions for term-based clause evaluation heuristics.

  Copyright 1998-2018 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Nov  7 21:37:27 CET 2018

-----------------------------------------------------------------------*/

#include "che_termweights.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

static PStack_p get_subterm_generalizing_vars(
   Term_p term, VarBank_p vars, NumTree_p* term_vars, long* fresh_var_code)
{
   PStack_p gen_vars;
   NumTree_p entry;
   Term_p fresh_var;

   entry = NumTreeFind(term_vars, term->entry_no);
   if (!entry) 
   {
      entry = NumTreeCellAllocEmpty();
      entry->key = term->entry_no;
      entry->val1.p_val = PStackAlloc();
      NumTreeInsert(term_vars, entry);
   }

   gen_vars = entry->val1.p_val;
   fresh_var = VarBankVarAssertAlloc(vars, *fresh_var_code, vars->sort_table->i_type);
   (*fresh_var_code) -= 2;
   PStackPushP(gen_vars, fresh_var);

   return gen_vars;
}

static PStack_p compute_subterms_generalizations(
   Term_p term, 
   VarBank_p vars,
   PStack_p all,
   NumTree_p* term_vars, // Term_p.entry_no -> PStack_p
   long* fresh_var_code)
{
   int i;
   int is_max;
   PStack_p gens;
   PStack_p sgen;
   PStack_p gen_vars;
   PDArray_p sgens;
   FixedDArray_p max;
   FixedDArray_p cur;
   Term_p copy;

   gens = PStackAlloc();

   gen_vars = get_subterm_generalizing_vars(
      term,vars,term_vars,fresh_var_code);
   PStackPushStack(gens, gen_vars);

   if (TermIsAnyVar(term)) 
   {
      return gens;
   }
 
   if (TermIsConst(term)) 
   {
      copy = TermTopAlloc(term->f_code,0);
      TermSetBank(copy, TermGetBank(term));
      PStackPushP(gens,copy);
      PStackPushP(all,copy);
      copy->properties = term->properties;
      copy->type = term->type;
      return gens;
   }

   assert(term->arity > 0);
      
   cur = FixedDArrayAlloc(term->arity);
   max = FixedDArrayAlloc(term->arity);
   sgens = PDArrayAlloc(term->arity,1);

   for (i=0; i<term->arity; i++)
   {
      sgen = compute_subterms_generalizations(
         term->args[i],vars,all,term_vars,fresh_var_code);
      assert(sgen->current >= 1);
      max->array[i] = sgen->current - 1;
      PDArrayAssignP(sgens,i,sgen);
   }

   int iter_counter = 0;
   for (is_max=TupleInit(cur); is_max; is_max=TupleNext(cur,max)) 
   {
      if (iter_counter > TERM_MAX_GENS) 
      {
         break;
      }
      copy = TermTopAlloc(term->f_code,term->arity);
      TermSetBank(copy, TermGetBank(term));
      for (i=0; i<term->arity; i++)
      {
         sgen = PDArrayElementP(sgens,i);
         copy->args[i] = PStackElementP(sgen,cur->array[i]);
      }
      copy->properties = term->properties;
      copy->type = term->type;
      PStackPushP(gens,copy);
      PStackPushP(all,copy);
      iter_counter++;
   }

   for (i=0; i<term->arity; i++)
   {
      sgen = PDArrayElementP(sgens,i);
      PStackFree(sgen);
   }
   FixedDArrayFree(max);
   FixedDArrayFree(cur);
   PDArrayFree(sgens);

   return gens;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ComputeSubtermsGeneralizations()
//
//   Compute generalizations of all subterms. The number of gens per 
//   term is limited by TERM_MAX_GENS.  
//
// Global Variables: -
//
// Side Effects    : allocates stack of terms
//
/----------------------------------------------------------------------*/

PStack_p ComputeSubtermsGeneralizations(Term_p term, VarBank_p vars)
{
   long fresh_var_code;
   NumTree_p term_vars;
   NumTree_p item;
   PStack_p all;
   PStack_p gens;
   PStack_p stack;

   all = PStackAlloc();

   term_vars = NULL;
   fresh_var_code = -2;

   gens = compute_subterms_generalizations(
      term,vars,all,&term_vars,&fresh_var_code);

   PStackFree(gens);
   stack = NumTreeTraverseInit(term_vars);
   while ((item=NumTreeTraverseNext(stack)))
   {
      PStackFree(item->val1.p_val);
   }
   NumTreeTraverseExit(stack);
   NumTreeFree(term_vars); 

   return all;
}

/*-----------------------------------------------------------------------
//
// Function: ComputeTopGeneralizations()
//
//   Compute top-level generalization "f(X1,..,Xn)" for each n-ary f 
//   from the term.
//
// Global Variables: -
//
// Side Effects    : allocates stack of terms
//
/----------------------------------------------------------------------*/

PStack_p ComputeTopGeneralizations(Term_p term, VarBank_p vars, Sig_p sig)
{
   int i;
   FunCode code;
   FixedDArray_p occurs;
   PStack_p stack, topgens;
   Term_p subterm, topgen;

   occurs = FixedDArrayAlloc(sig->f_count+1);
   for (code=1; code<=sig->f_count; code++)
   {
      occurs->array[code] = 0;
   }
   
   stack = PStackAlloc();
   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      if (TermIsFreeVar(subterm) || TermIsConst(subterm))
      {
         continue;
      }
      
      occurs->array[subterm->f_code] = 1;
      
      for(i=1; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }
   PStackFree(stack);

   topgens = PStackAlloc();
   for (code=1; code<=sig->f_count; code++)
   {
      if (!occurs->array[code]) 
      {
         continue;
      }
      
      topgen = TermTopAlloc(code, sig->f_info[code].arity);
      TermSetBank(topgen, TermGetBank(term));
      for (i=0; i<sig->f_info[code].arity; i++)
      {
         topgen->args[i] = VarBankVarAssertAlloc(vars, -2*(i+1), vars->sort_table->i_type); 
      }
      if (SigIsPredicate(sig,code)) 
      {
         TermCellSetProp(topgen, TPPredPos);
         topgen->type = vars->sort_table->bool_type;
      }
      else
      {
         topgen->type = vars->sort_table->i_type;
      }
      PStackPushP(topgens, topgen);
   }

   FixedDArrayFree(occurs);
   return topgens;
}

/*-----------------------------------------------------------------------
//
// Function: FreeGeneralizations()
//
//   Free the stack of terms and their top symbols.
//
// Global Variables: -
//
// Side Effects    : free memory
//
/----------------------------------------------------------------------*/

void FreeGeneralizations(PStack_p gens)
{
   Term_p gen;

   while (!PStackEmpty(gens))
   {
      gen = PStackPopP(gens);
      TermTopFree(gen);
   }
   PStackFree(gens);
}

/*-----------------------------------------------------------------------
//
// Function: TupleInit()
//
//   Used to traverse n-tupples from (0,0,...,0) to (n0,n1,...,nm) 
//   lexicographically.  This sets the tupple to zeros.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TupleInit(FixedDArray_p cur)
{
   int i;

   for (i=0; i<cur->size; i++) 
   {
      cur->array[i] = 0;
   }

   return (cur->size > 0); // should be always true
}

/*-----------------------------------------------------------------------
//
// Function: TupleNext()
//
//   Used to traverse n-tupples from (0,0,...,0) to (n0,n1,...,nm) 
//   lexicographically.  This makes `cur` the next tupple.  Maximal
//   values for each item (that is, those n's) is given in `max`.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int TupleNext(FixedDArray_p cur, FixedDArray_p max)
{
   int i;

   for (i=cur->size-1; i>=0; i--) 
   {
      if (cur->array[i] < max->array[i])
      {
         break;
      }
   }

   if (i == -1)
   {
      return 0; // cur is maximal value
   }

   cur->array[i]++;
   for (i=i+1; i<cur->size; i++) {
      cur->array[i] = 0;
   }

   return 1;
}

/*-----------------------------------------------------------------------
//
// Function: TuplePrint()
//
//   Print a tuple.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TuplePrint(FixedDArray_p t)
{
   int i;
   fprintf(GlobalOut, "(");
   for (i=0; i<t->size; i++) 
   {
      fprintf(GlobalOut, "%ld,", t->array[i]);
   }
   fprintf(GlobalOut, ")\n");
}

/*-----------------------------------------------------------------------
//
// Function: TBIncSubtermsFreqs()
//
//   Increase the frequency of all subterms of `term` in `freqs` where
//   `freqs` is a map from Term_p.entry_no to the frequency.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TBIncSubtermsFreqs(Term_p term, NumTree_p* freqs)
{
   int i;
   PStack_p stack;
   Term_p subterm;
   NumTree_p cell;

   stack = PStackAlloc();

   PStackPushP(stack, term);
   while (!PStackEmpty(stack))
   {
      subterm = PStackPopP(stack);
      if (TermIsFreeVar(subterm))
      {
         continue;
      }
         
      //subterm->freq++;
      cell = NumTreeFind(freqs, subterm->entry_no);
      if (cell)
      {
         cell->val1.i_val++;
      }
      else 
      {
         IntOrP val1;
         val1.i_val = 1;
         NumTreeStore(freqs, subterm->entry_no, val1, val1);
      }
      
      for(i=0; i<subterm->arity; i++)
      {
         PStackPushP(stack, subterm->args[i]);
      }
   }

   PStackFree(stack);
}

/*-----------------------------------------------------------------------
//
// Function: TBCountTermFreqs()
//
//   Iterates over a term bank and set the number of occurences 
//   (frequencies) for each term. Only top positions are processed
//   but other terms are also visited as all subterms are considered.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

NumTree_p TBCountTermFreqs(TB_p bank)
{
   PStack_p stack = PStackAlloc();
   Term_p term;
   int i;
   NumTree_p freqs = NULL;

   for(i=0; i<TERM_STORE_HASH_SIZE; i++)
   {      
      PStackPushP(stack, bank->term_store.store[i]);
      while(!PStackEmpty(stack))
      {
         term = PStackPopP(stack);
         if ((!term) || (!TermCellQueryProp(term,TPTopPos)))
         {
            continue;
         }
         TBIncSubtermsFreqs(term, &freqs);
         if(term)
         {
            PStackPushP(stack, term->lson);
            PStackPushP(stack, term->rson);
         }
      }
   }
   PStackFree(stack);

   return freqs;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

