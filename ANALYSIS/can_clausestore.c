/*-----------------------------------------------------------------------

File  : can_clausestore.h

Author: Stephan Schulz

Contents

  Functions dealing with compact clause representations.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jan 29 17:20:30 MET 1999
    New

-----------------------------------------------------------------------*/

#include "can_clausestore.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

long dummy;

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
// Function: CompClauseFree()
//
//   Release the memory taken by a compact clause, release term
//   references.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void CompClauseFree(CompClause_p clause, TB_p bank)
{
   if(clause->lit_terms)
   {
      CompClauseRemoveTerms(clause,bank);
   }
   PTreeFree(clause->g_parents);
   PTreeFree(clause->s_parents);
   CompClauseCellFree(clause);
}

/*-----------------------------------------------------------------------
//
// Function: CompClauseAddTerms()
//
//   Add the terms in term_clause to clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void CompClauseAddTerms(CompClause_p clause, Clause_p term_clause)
{
   int   i;
   Eqn_p literal;

   assert(!clause->lit_terms);
   assert(!clause->sign);
   clause->literal_no = ClauseLiteralNumber(term_clause);
   clause->lit_terms =
      SizeMalloc(2*clause->literal_no*sizeof(Term_p));
   clause->sign = SizeMalloc(clause->literal_no*sizeof(short));

   for(i=0, literal = term_clause->literals; literal; i++, literal =
     literal->next)
   {
      assert(i<clause->literal_no);

      clause->sign[i] = EqnIsPositive(literal);
      clause->lit_terms[(2*i)]  = literal->lterm;
      clause->lit_terms[(2*i)+1]= literal->rterm;
   }
}

/*-----------------------------------------------------------------------
//
// Function: CompClauseRemoveTerms()
//
//   Remove the terms from the clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void CompClauseRemoveTerms(CompClause_p clause, TB_p bank)
{
   assert(clause->lit_terms);
   assert(clause->sign);

   /* Terms will be garbage-collected automatically */
   SizeFree(clause->sign, clause->literal_no*sizeof(short));
   clause->sign = NULL;
   SizeFree(clause->lit_terms, 2*clause->literal_no*sizeof(Term_p));
   clause->lit_terms = NULL;
}


/*-----------------------------------------------------------------------
//
// Function: PackClause()
//
//   Generate a compact clause represention from the normal one.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompClause_p PackClause(Clause_p clause)
{
   CompClause_p handle = CompClauseCellAlloc();

   handle->properties = CPIgnoreProps;
   handle->lit_terms = NULL;
   handle->sign = NULL;
   handle->ident = clause->ident;
   handle->g_parents = NULL;
   handle->s_parents = NULL;
   handle->stats.subsumed = 0;
   handle->ext_ident = 0;
   CompClauseAddTerms(handle, clause);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: UnpackClause()
//
//   Given a compact clause, create the corresponding normal clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p UnpackClause(CompClause_p clause, TB_p bank)
{
   Eqn_p    list=NULL, *eqn = &list;
   Clause_p handle;
   int i;

   for(i=0; i<clause->literal_no; i++)
   {
      *eqn = EqnAlloc(clause->lit_terms[(2*i)],
            clause->lit_terms[(2*i)+1],
            bank, clause->sign[i]);
      eqn = &((*eqn)->next);
   }
   handle = ClauseAlloc(list);
   handle->ident = clause->ident;
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: CompactifyClause()
//
//   As PackClause(), but destroy original.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

CompClause_p CompactifyClause(Clause_p clause)
{
   CompClause_p handle = PackClause(clause);

   ClauseFree(clause);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: UnCompactifyClause()
//
//   As UnpackClause(), but destroy original.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p UnCompactifyClause(CompClause_p clause, TB_p bank)
{
   Clause_p handle = UnpackClause(clause, bank);

   CompClauseFree(clause, bank);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: CompClausePrint()
//
//   Print a compact clause. Not the best way, but the easiest!
//
// Global Variables: -
//
// Side Effects    : Output, memory operations.
//
/----------------------------------------------------------------------*/

void CompClausePrint(FILE* out, CompClause_p compact, TB_p bank, bool
           full_terms)
{
   Clause_p clause;

   clause = UnpackClause(compact, bank);
   ClausePrint(out, clause, full_terms);
   ClauseFree(clause);
}


/*-----------------------------------------------------------------------
//
// Function: CompClausePCLPrint()
//
//   Print the clause in PCL format, i.e. as a literal list.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations.
//
/----------------------------------------------------------------------*/

void CompClausePCLPrint(FILE* out, CompClause_p compact, TB_p bank)
{
   Clause_p clause;

   clause = UnpackClause(compact, bank);
   ClausePCLPrint(out, clause, true);
   ClauseFree(clause);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


