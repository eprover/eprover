/*-----------------------------------------------------------------------

File  : cpr_dpllformula.h

Author: Stephan Schulz

Contents

  Base data structure for representing the state of a propositional
  formula (in CNF) for a DPLL procedure. I'm doing this for the first
  time, so it probably is sub-perfect....

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri May  2 17:50:53 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CPR_DPLLFORMULA

#define CPR_DPLLFORMULA

#include <cpr_propclauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct atom_cell
{
   long pos_occur;
   long neg_occur;
   PTree_p pos_active;
   PTree_p neg_active;
}AtomCell, *Atom_p;

typedef struct dpllrep_cell
{
   PropSig_p sig;
   PStack_p  clauses;
   long      atom_no;
   Atom_p    atoms;
}DPLLFormulaCell, *DPLLFormula_p;


#define DEFAULT_ATOM_NUMBER 500;
#define ATOM_GROWTH_FACTOR  1.5


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DPLLFormulaCellAlloc() (DPLLFormulaCell*)SizeMalloc(sizeof(DPLLFormulaCell))
#define DPLLFormulaCellFree(junk)            SizeFree(junk, sizeof(DPLLFormulaCell))

DPLLFormula_p DPLLFormulaAlloc(void);
void DPLLFormulaFree(DPLLFormula_p junk);
void DPLLFormulaPrint(FILE* out,DPLLFormula_p form, DPLLOutputFormat format,
            bool print_atoms);
void DPLLRegisterClauseLiteral(DPLLFormula_p form,
                DPLLClause_p clause,
                PLiteralCode lit);
void DPLLFormulaInsertClause(DPLLFormula_p form, DPLLClause_p clause);
void DPLLFormulaParseLOP(Scanner_p in, Sig_p sig, DPLLFormula_p form);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





