/*-----------------------------------------------------------------------

File  : ccl_def_handling.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  Handling of clausal definitons as used (up to now
  implicietely) in splitting, i.e. data structures associating a
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
   
   assert(bank);assert(pred > 0);
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
      
      lside = TermDefaultCellAlloc();
      lside->f_code = pred;
      lside->arity = arity;
      lside->args = TermArgArrayAlloc(arity);
      for(i=0; i<arity; i++)
      {
	 lside->args[i] = PStackElementP(split_vars, i);
      }
   }   
   lside = TBTermTopInsert(bank, lside);
   res = EqnAlloc(lside, bank->true_term, bank, positive);
   EqnSetProp(res, EPIsSplitLit);
   
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: GetDefinition()
//
//   Given a literal list, return the explicit remainder of the
//   definition (if a new definition is necessary). Reuses or discards
//   the literal list! Also return the predicate code (via def_pred).
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

Clause_p GetDefinition(DefStore_p store, Eqn_p litlist, 
                       FunCode *def_pred, bool fresh, long *apply_id)
{
   Clause_p res, def_clause;
   Eqn_p def_lit;

   def_clause = ClauseAlloc(EqnListFlatCopy(litlist));
   def_clause->weight = ClauseStandardWeight(def_clause);
   ClauseSubsumeOrderSortLits(def_clause);

   assert(litlist);
   if(fresh)
   {
      *def_pred     = SigGetNewPredicateCode(store->terms->sig, 0);

      def_lit       = GenDefLit(litlist->bank, *def_pred, true, NULL);
      def_lit->next = litlist;
      res           = ClauseAlloc(def_lit);      

      DocIntroSplitDefDefault(def_clause, def_lit);
      DocIntroSplitDefRestDefault(res, def_clause);
      *apply_id       = def_clause->ident;

      ClauseFree(def_clause);
   }
   else
   {
      Clause_p variant;
      
      variant = ClauseSetFindVariantClause(store->def_clauses,
                                           def_clause);      
      if(variant)
      {
         NumTree_p assoc = NumTreeFind(&(store->def_assocs), 
                                       variant->ident);
         assert(assoc);
         res       = NULL; /* Clause already exists */
         *def_pred = assoc->val1.i_val;
         *apply_id = variant->ident;
         ClauseFree(def_clause);
         EqnListFree(litlist);
      }
      else
      {
         IntOrP def_pred_store;

         *def_pred     = SigGetNewPredicateCode(store->terms->sig, 0);
         def_lit       = GenDefLit(litlist->bank, *def_pred, true, NULL);
         def_lit->next = litlist;
         res           = ClauseAlloc(def_lit);      

         DocIntroSplitDefDefault(def_clause, def_lit);
         DocIntroSplitDefRestDefault(res, def_clause);
         *apply_id     = def_clause->ident;

         def_pred_store.i_val = *def_pred;       
         NumTreeStore(&(store->def_assocs),
                      def_clause->ident, 
                      def_pred_store, 
                      def_pred_store);
         ClauseSetIndexedInsertClause(store->def_clauses, 
                                      def_clause);
      }
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


