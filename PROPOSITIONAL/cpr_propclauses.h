/*-----------------------------------------------------------------------

File  : cpr_propclauses.h

Author: Stephan Schulz

Contents

  Datatypes for the efficient representation of propositional clauses
  for a DPLL procedure.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 23 12:10:35 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CPR_PROPCLAUSES

#define CPR_PROPCLAUSES

#include <ccl_clauses.h>
#include <cpr_propsig.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   DPLLOutNoFormat,
   DPLLOutLOP,
   DPLLOutDimacs
}DPLLOutputFormat;

typedef struct dpll_clause_cell
{
   unsigned long mem_size;
   unsigned long lit_no;
   unsigned long active_no;
   PLiteralCode  *literals;
}DPLLClauseCell, *DPLLClause_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define DPLLClauseCellAlloc() (DPLLClauseCell*)SizeMalloc(sizeof(DPLLClauseCell))
#define DPLLClauseCellFree(junk)            SizeFree(junk, sizeof(DPLLClauseCell))

#define DPLLClauseIsUnit(clause) ((clause)->active_no==1)

void         DPLLClauseFree(DPLLClause_p junk);
DPLLClause_p DPLLClauseFromClause(PropSig_p psig, Clause_p clause);
bool         DPLLClauseNormalize(DPLLClause_p clause);
void         DPLLClausePrintLOP(FILE* out, PropSig_p psig, DPLLClause_p clause);
void         DPLLClausePrintDimacs(FILE* out, DPLLClause_p clause);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





