/*-----------------------------------------------------------------------

File  : pcl_miniclauses.c

Author: Stephan Schulz

Contents

  Functions dealing with minimal (compact clause representations.

  Copyright 1998, 1999, 2002 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jan 29 17:20:30 MET 1999
    New

-----------------------------------------------------------------------*/

#include "pcl_miniclauses.h"



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
// Function: MiniClauseFree()
//
//   Release the memory taken by a compact clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void MiniClauseFree(MiniClause_p clause)
{
   if(clause->lit_terms)
   {
      SizeFree(clause->sign, clause->literal_no*sizeof(short));
      clause->sign = NULL;
      SizeFree(clause->lit_terms, 2*clause->literal_no*sizeof(Term_p));
      clause->lit_terms = NULL;
   }
   MiniClauseCellFree(clause);
}

/*-----------------------------------------------------------------------
//
// Function: MiniClauseAddTerms()
//
//   Add the terms in term_clause to clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void MiniClauseAddTerms(MiniClause_p clause, Clause_p term_clause)
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
      clause->lit_terms[(2*i)]   = literal->lterm;
      clause->lit_terms[(2*i)+1] = literal->rterm;
   }
}



/*-----------------------------------------------------------------------
//
// Function: ClauseToMiniClause()
//
//   Generate a compact clause represention from the normal one.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

MiniClause_p ClauseToMiniClause(Clause_p clause)
{
   MiniClause_p handle = MiniClauseCellAlloc();
   int   i;
   Eqn_p literal;

   /* handle->properties = CPIgnoreProps; */
   handle->lit_terms  = NULL;
   handle->sign       = NULL;
   handle->literal_no = ClauseLiteralNumber(clause);

   if(handle->literal_no)
   {
      handle->lit_terms = SizeMalloc(2*ClauseLiteralNumber(clause)*sizeof(Term_p));
      handle->sign = SizeMalloc(ClauseLiteralNumber(clause)*sizeof(short));

      for(i=0, literal = clause->literals; literal; i++, literal =
        literal->next)
      {
    assert(i<handle->literal_no);
    handle->sign[i] = EqnIsPositive(literal);
    handle->lit_terms[(2*i)]    = literal->lterm;
    handle->lit_terms[(2*i)+1]  = literal->rterm;
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: MiniClauseToClause()
//
//   Given a compact clause, create the corresponding normal clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p MiniClauseToClause(MiniClause_p clause, TB_p bank)
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
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: MinifyClause()
//
//   As ClauseToMiniClause(), but destroy original.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

MiniClause_p MinifyClause(Clause_p clause)
{
   MiniClause_p handle = ClauseToMiniClause(clause);

   ClauseFree(clause);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: UnMinifyClause()
//
//   As MiniClauseToClause(), but destroy original.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

Clause_p UnMinifyClause(MiniClause_p clause, TB_p bank)
{
   Clause_p handle = MiniClauseToClause(clause, bank);

   MiniClauseFree(clause);

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: MiniClausePrint()
//
//   Print a compact clause. Not the best way, but the easiest!
//
// Global Variables: -
//
// Side Effects    : Output, memory operations.
//
/----------------------------------------------------------------------*/

void MiniClausePrint(FILE* out, MiniClause_p compact, TB_p bank, bool
           full_terms)
{
   Clause_p clause;

   clause = MiniClauseToClause(compact, bank);
   ClausePrint(out, clause, full_terms);
   ClauseFree(clause);
}


/*-----------------------------------------------------------------------
//
// Function: MiniClausePCLPrint()
//
//   Print the clause in PCL format, i.e. as a literal list.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations.
//
/----------------------------------------------------------------------*/

void MiniClausePCLPrint(FILE* out, MiniClause_p compact, TB_p bank)
{
   Clause_p clause;

   clause = MiniClauseToClause(compact, bank);
   ClausePCLPrint(out, clause, true);
   ClauseFree(clause);
}


/*-----------------------------------------------------------------------
//
// Function: MiniClauseTSTPCorePrint()
//
//   Print the core clause in TSTP format, i.e. as a literal list.
//
// Global Variables: -
//
// Side Effects    : Output, memory operations.
//
/----------------------------------------------------------------------*/

void MiniClauseTSTPCorePrint(FILE* out, MiniClause_p compact, TB_p bank)
{
   Clause_p clause;

   clause = MiniClauseToClause(compact, bank);
   ClauseTSTPCorePrint(out, clause, true);
   ClauseFree(clause);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


