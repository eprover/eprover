/*-----------------------------------------------------------------------

File  : cpr_propclauses.h

Author: Stephan Schulz

Contents
 
  Datatypes for the efficient representation of propositional clauses
  for a DPLL procedure.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Wed Apr 23 12:10:35 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CPR_PROPCLAUSES

#define CPR_PROPCLAUSES

#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct lit_rep_cell
{
   long  literal;
   bool   active;
}LitRepCell, *LitRep_p;

typedef struct dpll_clause_cell
{
   unsigned long lit_no;
   unsigned long active_no;
   LitRep_p      literals;
}DPLLClauseCell, *DPLLClause_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

DPLLClause_p DPLLClauseFromClause(PropSig_p psig, Clause_p clause);
void         DPLLClausePrintLOP(PropSig_p psig, DPLLClause_p clause);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





