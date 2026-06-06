/*-----------------------------------------------------------------------

  File  : ccl_gd_transformation.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Function implementing a TWEE-style direct goal
  transformation (by adding equational definitions that reduce goal
  ground terms to (usually new) constants.

  This goes from clause level to signature level - I put it together
  here to keep things under control...

  Copyright 2026 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created: Sun May 31 17:42:01 CEST 2026

-----------------------------------------------------------------------*/

#include <ccl_gd_transformation.h>


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
// Function: gd_def_nf()
//
//   Compute the normal-form of term with respect to defs.
//
// Global Variables: -
//
// Side Effects    : Creates new term in term bank
//
/----------------------------------------------------------------------*/

Term_p gd_def_nf(TB_p terms, Term_p term, NumTree_p *defs)
{
   int i;
   Term_p def_lhs;
   NumTree_p cell;

   if(TermIsConst(term))
   {
      return term;
   }
   cell = NumTreeFind(defs, term->entry_no);
   if(cell)
   {
      return  cell->val1.p_val;
   }
   def_lhs = TermTopCopy(term);
   for(i=0; i<term->arity; i++)
   {
      def_lhs->args[i] = gd_def_nf(terms, term->args[i], defs);
   }
   def_lhs = TBTermTopInsert(terms, def_lhs);
   cell = NumTreeFind(defs, def_lhs->entry_no);
   if(cell)
   {
      return  cell->val1.p_val;
   }
   return def_lhs;
}

/*-----------------------------------------------------------------------
//
// Function: gd_term_rek_define()
//
//   Conditionally (if it does not already exist) add a definiton for
//   term -> New Constant.
//
//   Definitions are stored in defs, with cell->key being the entry_no
//   of the LHS, and cell->val1.pval pointing to the RHS. The
//   defining clause is added to clauses.
//
// Global Variables: -
//
// Side Effects    : As described
//
/----------------------------------------------------------------------*/

long gd_term_define(TB_p terms, Term_p term,
                    NumTree_p *defs,
                    ClauseSet_p clauses)
{
   Term_p rhs, lhs;
   FunCode new_const;
   Eqn_p def_eqn;
   Clause_p clause;

   if(TermIsConst(term) || NumTreeFind(defs, term->entry_no))
   {
      return 0;
   }
   lhs = gd_def_nf(terms, term, defs);
   new_const = SigGetNewTypedDefCode(terms->sig, NULL, 0, GetReturnSort(term->type));
   rhs = TermConstCellAlloc(new_const);
   rhs = TBTermTopInsert(terms, rhs);
   NumTreeStore(defs, lhs->entry_no,
                ((IntOrP){.p_val=rhs}),
                ((IntOrP){.p_val=NULL}));
   def_eqn = EqnAlloc(lhs, rhs, terms, true);
   /* fprintf(GlobalOut, "New definition: "); */
   /* EqnPrint(GlobalOut,def_eqn,false,true); */
   /* fprintf(GlobalOut, "\n"); */
   assert(!def_eqn->next);
   clause = ClauseAlloc(def_eqn);
   ClausePushDerivation(clause, DCIntroDef, NULL, NULL);
   ClauseSetInsert(clauses, clause);

   return 1;
}

/*-----------------------------------------------------------------------
//
// Function: gd_term_rek_define()
//
//   Add definitions for the normalform of term (with respect to defs)
//   and all its subterms to clauses. Returns number of definitions
//   added.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long gd_term_rek_define(TB_p terms, Term_p term,
                        NumTree_p *defs,
                        ClauseSet_p clauses)
{
   long res = 0;

   if(TermIsConst(term))
   {
      return res;
   }
   assert(TermIsGround(term));

   int i;

   for(i=0; i<term->arity; i++)
   {
      res += gd_term_rek_define(terms, term->args[i], defs, clauses);
   }

   res += gd_term_define(terms, term, defs, clauses);
   return res;
}




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: ClauseSetGDTransform()
//
//   Perform a Twee-style goal-direct transformation, by adding
//   equations (unit clauses) that will reduce some or all ground
//   subterms from conjecture clauses to fresh constants. Returns the
//   number of new defintions introduced.
//
// Global Variables: -
//
// Side Effects    : Memory operations (adds clauses and symbols).
//
/----------------------------------------------------------------------*/

long ClauseSetGDTransform(TB_p terms,
                          ClauseSet_p clauses,
                          bool add_goal_defs_pos,
                          bool add_goal_defs_neg,
                          bool add_goal_defs_subterms)
{
   long res = 0;
   Clause_p handle;
   PTree_p goal_terms = NULL, cell;
   PStack_p iter_stack;
   Term_p term;
   NumTree_p defs = NULL;

   for(handle = clauses->anchor->succ;
       handle!=clauses->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         ClauseCollectGroundTerms(handle, &goal_terms,
                                  add_goal_defs_pos,
                                  add_goal_defs_neg,
                                  add_goal_defs_subterms);
      }
   }
   iter_stack = PTreeTraverseInit(goal_terms);

   while((cell = PTreeTraverseNext(iter_stack)))
   {
      term = cell->key;
      assert(TermIsGround(term));
      if(!NumTreeFind(&defs, term->entry_no) && !TermIsConst(term))
      {
         if(add_goal_defs_subterms)
         {
            res += gd_term_rek_define(terms, term, &defs, clauses);
         }
         else
         {
            res += gd_term_define(terms, term, &defs, clauses);
         }
      }
   }
   PTreeTraverseExit(iter_stack);
   PTreeFree(goal_terms);
   NumTreeFree(defs);

   return res;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
