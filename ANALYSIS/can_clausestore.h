/*-----------------------------------------------------------------------

File  : can_clausestore.h

Author: Stephan Schulz

Contents

  Data type for compact storing of clauses, indexed by identifier, for
  analysis purposes.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:28:25 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CAN_CLAUSESTORE

#define CAN_CLAUSESTORE

#include <ccl_clauses.h>
#include <can_clausestats.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Represent a clause as compact as possible for analysis
   purposes. You need some context to interprete this, in particular
   the term bank. */

typedef struct compact_clause_cell
{
   FormulaProperties properties;
   long              ident;
   long              ext_ident;   /* Ident of the current version of
                                     the clause for PCL generation */
   short            literal_no;
   short            *sign; /* For literals */
   Term_p           *lit_terms; /* Literals are just pairs of terms */
   PTree_p          g_parents; /* Generating parents */
   PTree_p          s_parents; /* Simplifying parents */
   ClauseStatsCell  stats;
}CompClauseCell, *CompClause_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define CompClauseCellAlloc()\
        (CompClauseCell*)SizeMalloc(sizeof(CompClauseCell))
#define CompClauseCellFree(junk)\
        SizeFree(junk, sizeof(CompClauseCell))

void         CompClauseFree(CompClause_p clause, TB_p bank);

void CompClauseAddTerms(CompClause_p clause, Clause_p term_clause);
void CompClauseRemoveTerms(CompClause_p clause, TB_p bank);

CompClause_p PackClause(Clause_p clause);
Clause_p     UnpackClause(CompClause_p clause, TB_p bank);

CompClause_p CompactifyClause(Clause_p clause);
Clause_p     UnCompactifyClause(CompClause_p clause, TB_p bank);

void CompClausePrint(FILE* out, CompClause_p compact, TB_p bank, bool
           full_terms);

void CompClausePCLPrint(FILE* out, CompClause_p compact, TB_p bank);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
