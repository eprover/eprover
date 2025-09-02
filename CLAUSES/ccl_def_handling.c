/*-----------------------------------------------------------------------

File  : ccl_def_handling.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Handling of clausal definitions as used (up to now
  implicitely) in splitting, i.e. data structures associating a
  clause with a fresh constant predicate symbol or literal.

  Copyright 2006 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun  4 20:31:23 EEST 2006
    New

-----------------------------------------------------------------------*/

#include "ccl_def_handling.h"



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
// Function: DefStoreAlloc()
//
//   Return an initialized definitions storage object. Note that the
//   FVIndex in def_clauses still has to be set (this is an inherited
//   uglyness I'll fix soon).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

DefStore_p DefStoreAlloc(TB_p terms)
{
   DefStore_p store = DefStoreCellAlloc();

   store->terms       = terms;
   store->def_clauses = ClauseSetAlloc();
   store->def_assocs  = NULL;
   store->def_archive = FormulaSetAlloc();

   return store;
}


/*-----------------------------------------------------------------------
//
// Function: DefStoreFree()
//
//   Free a definition storage object and all data it is responsible
//   for (includes the FVIndex of def_clauses, but not the term
//   bank).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DefStoreFree(DefStore_p junk)
{
   ClauseSetFree(junk->def_clauses);
   NumTreeFree(junk->def_assocs);
   FormulaSetFree(junk->def_archive);
   DefStoreCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: GenDefLit()
//
//   Generate a definition literal with terms from bank.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

Eqn_p GenDefLit(TB_p bank, FunCode pred, bool positive,
                PStack_p split_vars)
{
   Term_p lside;
   Eqn_p  res;

   assert(bank);
   assert(pred > 0);
   assert((split_vars &&
           (SigFindArity(bank->sig, pred) == PStackGetSP(split_vars)))
          ||
          (!split_vars && (SigFindArity(bank->sig, pred)==0)));

   if(!split_vars || PStackEmpty(split_vars))
   {
      lside = TermConstCellAlloc(pred);
   }
   else
   {
      int arity = PStackGetSP(split_vars), i;

      lside = TermDefaultCellArityAlloc(arity);
      lside->f_code = pred;
      lside->arity = arity;

      for(i = 0; i < arity; ++i)
      {
         lside->args[i] = PStackElementP(split_vars, i);
      }
   }
   lside->type = bank->sig->type_bank->bool_type;
   lside = TBTermTopInsert(bank, lside);
   res = EqnAlloc(lside, bank->true_term, bank, positive);
   EqnSetProp(res, EPIsSplitLit);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: GetClauseDefinition()
//
//   Given a literal list and the definition predicate, generate one
//   of the two clauses the equivalence definition splits into (namely
//   the one we need to add for splitting). This recycles the literal
//   list!
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

Clause_p GetClauseDefinition(Eqn_p litlist, FunCode def_pred, WFormula_p parent)
{
   Clause_p res;
   Eqn_p    def_lit;

   assert(litlist);
   assert(def_pred > 0);

   def_lit = GenDefLit(litlist->bank, def_pred, true, NULL);
   def_lit->next = litlist;
   res           = ClauseAlloc(def_lit);

   ClausePushDerivation(res, DCSplitEquiv, parent, NULL);

   DocIntroSplitDefRestDefault(res, parent);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GetFormulaDefinition()
//
//   Given a literal list and the definition predicate, generate the
//   equivalent defintion. This one leaves the literal list alone!
//
// Global Variables: -
//
// Side Effects    : May create output, memory ops
//
/----------------------------------------------------------------------*/

WFormula_p GetFormulaDefinition(Eqn_p litlist, FunCode def_pred)
{
   WFormula_p res;
   Eqn_p      def_lit;
   Clause_p   def_clause;
   TFormula_p def, lit;

   assert(litlist);
   assert(def_pred > 0);

   def_lit = GenDefLit(litlist->bank, def_pred, true, NULL);
   EqnFlipProp(def_lit, EPIsPositive);

   def_clause = ClauseAlloc(EqnListFlatCopy(litlist));

   lit = TFormulaLitAlloc(def_lit);
   EqnFree(def_lit);

   def = TFormulaClauseClosedEncode(litlist->bank, def_clause);
   def = TFormulaFCodeAlloc(litlist->bank, litlist->bank->sig->equiv_code, lit, def);
   res = WTFormulaAlloc(litlist->bank, def);

   ClauseFree(def_clause);

   DocIntroSplitDefDefault(res);
   WFormulaPushDerivation(res, DCIntroDef, NULL, NULL);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GetDefinitions()
//
//   Given a literal list, provide (optionally) the full definition and
//   the clause equivalent to the non-applied direction of the
//   definition. Return defined predicate.
//
//   If fresh is true, always return a fresh definition and do not
//   insert the clause/predicate association into the store. If fresh
//   is false, check if it is a variant of a known definiton and
//   return the corresponding symbol. If not, store the new
//   association.
//
// Global Variables: -
//
// Side Effects    : May extend the clause store.
//
/----------------------------------------------------------------------*/

FunCode GetDefinitions(DefStore_p store, Eqn_p litlist,
                       WFormula_p* res_form, Clause_p* res_clause,
                       bool fresh)
{
   Clause_p   def_clause;
   FunCode    def_pred = 0;

   assert(litlist);

   *res_form   = NULL;
   *res_clause = NULL;

   if(fresh)
   {
      def_pred    = SigGetNewPredicateCode(store->terms->sig, 0);
      SigDeclareType(store->terms->sig, def_pred, store->terms->sig->type_bank->bool_type);

      *res_form = GetFormulaDefinition(litlist, def_pred);
      FormulaSetInsert(store->def_archive, *res_form);
      *res_clause = GetClauseDefinition(litlist, def_pred, *res_form);
   }
   else
   {
      Clause_p variant;

      def_clause = ClauseAlloc(EqnListFlatCopy(litlist));
      def_clause->weight = ClauseStandardWeight(def_clause);
      ClauseSubsumeOrderSortLits(def_clause);

      variant = ClauseSetFindVariantClause(store->def_clauses,
                                           def_clause);
      if(variant)
      {
         NumTree_p assoc = NumTreeFind(&(store->def_assocs),
                                       variant->ident);
         assert(assoc);
         *res_clause = NULL; /* Clause already exists */
         *res_form = assoc->val2.p_val;
         def_pred = assoc->val1.i_val;
         ClauseFree(def_clause);
         EqnListFree(litlist);
      }
      else
      {
         IntOrP def_pred_store, def_form_store;

         def_pred = SigGetNewPredicateCode(store->terms->sig, 0);
         SigDeclareType(store->terms->sig, def_pred, store->terms->sig->type_bank->bool_type);
         *res_form = GetFormulaDefinition(litlist, def_pred);
         FormulaSetInsert(store->def_archive, *res_form);
         *res_clause = GetClauseDefinition(litlist, def_pred, *res_form);

         def_pred_store.i_val = def_pred;
         def_form_store.p_val = *res_form;
         NumTreeStore(&(store->def_assocs),
                      def_clause->ident,
                      def_pred_store,
                      def_form_store);
         assert(def_clause->weight == ClauseStandardWeight(def_clause));
         ClauseSetIndexedInsertClause(store->def_clauses,
                                      def_clause);
      }
   }
   return def_pred;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
