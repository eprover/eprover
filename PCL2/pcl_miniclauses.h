/*-----------------------------------------------------------------------

File  : pcl_miniclauses.h

Author: Stephan Schulz

Contents

  Maximal compact representation for clauses, to be used in compact
  pcl listings (and possibly wherever elese needed). Adaptded from
  can_clausestore.h.

  Copyright 1998, 1999, 2002 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jul 10 20:50:02 MEST 2002
    Borrowed and modifed (extensively) from clausestore.[ch]

-----------------------------------------------------------------------*/

#ifndef PCL_MINICLAUSES

#define PCL_MINICLAUSES

#include <ccl_clauses.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Represent a clause as compact as possible. You need some context to
   interprete this, in particular the term bank. */

typedef struct mini_clause_cell
{
   /* ClauseProperties properties; */
   short            literal_no;
   short            *sign; /* For literals */
   Term_p           *lit_terms; /* Literals are just pairs of terms */
}MiniClauseCell, *MiniClause_p;




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define MiniClauseCellAlloc()\
        (MiniClauseCell*)SizeMalloc(sizeof(MiniClauseCell))
#define MiniClauseCellFree(junk)\
        SizeFree(junk, sizeof(MiniClauseCell))

void    MiniClauseFree(MiniClause_p clause);

MiniClause_p ClauseToMiniClause(Clause_p clause);
Clause_p     MiniClauseToClause(MiniClause_p clause, TB_p bank);

MiniClause_p MinifyClause(Clause_p clause);
Clause_p     UnMinifyClause(MiniClause_p clause, TB_p bank);

void MiniClausePrint(FILE* out, MiniClause_p compact, TB_p bank, bool
           full_terms);

void MiniClausePCLPrint(FILE* out, MiniClause_p compact, TB_p bank);
void MiniClauseTSTPCorePrint(FILE* out, MiniClause_p compact, TB_p bank);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





