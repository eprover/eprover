/*-----------------------------------------------------------------------

File  : ccl_subterm_index.c

Author: Stephan Schulz (schulz@eprover.org)

Contents

  An index mapping subterms to occurances in clauses.

  Copyright 2010 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 14 09:27:17 CEST 2010
    New

-----------------------------------------------------------------------*/

#include "ccl_subterm_index.h"



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
// Function: term_collect_idx_subterms()
//
//
//   Collect all non-variable subterms in term either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

static long term_collect_idx_subterms(Term_p term, PTree_p *rest,
                                      PTree_p *full, bool restricted,
                                      bool lambda_demod)
{
   long res = 0;
   int i;
   PTree_p *tree;

   if(TermIsFreeVar(term))
   {
      return 0;
   }

   tree = restricted?rest:full;

   if((!lambda_demod || TermIsDBClosed(term)) && 
       PTreeStore(tree, term))
   {
      res++;
   }
   if(!lambda_demod && !TermIsLambda(term))
   {
      for(i=0; i<term->arity; i++)
      {
         res += term_collect_idx_subterms(term->args[i], rest, full, false, true);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: eqn_collect_idx_subterms()
//
//   Collect all non-variable subterms in eqn either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Via term_collect_idx_subterms()
//
/----------------------------------------------------------------------*/

static long eqn_collect_idx_subterms(Eqn_p eqn, PTree_p *rest, 
                                     PTree_p *full, bool lambda_demod)
{
   long res = 0;
   bool restricted_rw = EqnIsMaximal(eqn) && EqnIsPositive(eqn) && EqnIsOriented(eqn);

   res += term_collect_idx_subterms(eqn->lterm, rest, full, restricted_rw, lambda_demod);
   res += term_collect_idx_subterms(eqn->rterm, rest, full, false, lambda_demod);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: subterm_index_insert_set()
//
//   Insert all the subterm/clause relationships in set (represented
//   as a PTree) into the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void subterm_index_insert_set(SubtermIndex_p index,
                                     Clause_p clause,
                                     PTree_p terms, bool restricted)
{
   PStack_p stack = PTreeTraverseInit(terms);
   PTree_p   cell;

   while((cell = PTreeTraverseNext(stack)))
   {
      SubtermIndexInsertOcc(index, clause, cell->key, restricted);
   }
   PTreeTraverseExit(stack);
}


/*-----------------------------------------------------------------------
//
// Function: subterm_index_delete_set()
//
//   Delete all the subterm/clause relationships in set (represented
//   as a PTree) into the index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static void subterm_index_delete_set(SubtermIndex_p index,
                                     Clause_p clause,
                                     PTree_p terms, bool restricted)
{
   PStack_p stack = PTreeTraverseInit(terms);
   PTree_p   cell;

   while((cell = PTreeTraverseNext(stack)))
   {
      SubtermIndexDeleteOcc(index, clause, cell->key, restricted);
   }
   PTreeTraverseExit(stack);
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



/*-----------------------------------------------------------------------
//
// Function: SubtermIndexInsertOcc()
//
//   Insert a  given occurance of a subterm into the index. Return
//   true if it was new, false if it already existed.
//
// Global Variables: -
//
// Side Effects    : Memory operatios
//
/----------------------------------------------------------------------*/
bool SubtermIndexInsertOcc(SubtermIndex_p index, Clause_p clause,
                           Term_p term, bool restricted)
{
   FPTree_p     fp_node;
   SubtermOcc_p subterm_node;
   PTree_p      *root;

   fp_node      = FPIndexInsert(index, term);
   subterm_node = SubtermTreeInsertTerm((void*)&(fp_node->payload), term);
   root         = restricted?&(subterm_node->pl.occs.rw_rest):&(subterm_node->pl.occs.rw_full);

   return PTreeStore(root, clause);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermIndexDeleteOcc()
//
//   Delete a given occurance of a subterm from the index. Return true
//   if the clause existed, false otherwise.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

bool SubtermIndexDeleteOcc(SubtermIndex_p index, Clause_p clause,
                           Term_p term, bool restricted)
{
   FPTree_p     fp_node;
   bool         res;

   fp_node      = FPIndexFind(index, term);
   if(!fp_node)
   {
      return false;
   }
   res =  SubtermTreeDeleteTermOcc((void*)&(fp_node->payload), term,
                                   clause, restricted);

   if(fp_node->payload == NULL)
   {
      FPIndexDelete(index, term);
   }
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: ClauseCollectIdxSubterms()
//
//   Collect all non-variable subterms in clause either into rest or
//   full (rest for "restricted rewriting" terms, full for the "full
//   rewriting" terms).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseCollectIdxSubterms(Clause_p clause,
                              PTree_p *rest,
                              PTree_p *full,
                              bool lambda_demod)
{
   long res = 0;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += eqn_collect_idx_subterms(handle, rest, full, lambda_demod);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SubtermIndexInsertClause()
//
//   Insert a clause into the subterm index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SubtermIndexInsertClause(SubtermIndex_p index, Clause_p clause,
                              bool lambda_demod)
{
   PTree_p rest=NULL, full=NULL;

   ClauseCollectIdxSubterms(clause, &rest, &full, lambda_demod);

   subterm_index_insert_set(index, clause, rest, true);
   subterm_index_insert_set(index, clause, full, false);

   PTreeFree(rest);
   PTreeFree(full);
}


/*-----------------------------------------------------------------------
//
// Function: SubtermIndexDeleteClause()
//
//   Delete a clause from the subterm index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SubtermIndexDeleteClause(SubtermIndex_p index, Clause_p clause, bool lambda_demod)
{
   PTree_p rest=NULL, full=NULL;

   ClauseCollectIdxSubterms(clause, &rest, &full, lambda_demod);

   subterm_index_delete_set(index, clause, rest, true);
   subterm_index_delete_set(index, clause, full, false);

   PTreeFree(rest);
   PTreeFree(full);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


