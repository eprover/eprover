/*-----------------------------------------------------------------------

  File  : ccl_arithnorm.h

  Author: Florian Knoch and Lukas Naatz

  Contents
      Definitions for arithmetic normalisation


  Copyright 2017 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:
<1> Tue Mar 23 15:29:05 MET 2021
    New

-----------------------------------------------------------------------*/

#include <ccl_formulasets.h>
#include <cte_termbanks.h>
#include <ccl_tformulae.h>
#include <cte_functypes.h>
#include <cte_simpletypes.h>
#include <ccl_derivation.h>
#include <ccl_clauses.h>
#include <ccl_eqn.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct acnormalize_cell {
	TFormula_p acterm;
	bool isground;
	struct acnormalize_cell *succ; 
} ACNormalizeCell, *ACNorm_p;

// header cell for both Termtypes, a merge sort would be ideal for more types 
typedef struct acstruct_cell {
	ACNorm_p groundterms;
	ACNorm_p nongroundterms;
} ACNormalizeStruct, *ACStruct_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

void FormulaSetArithNorm(FormulaSet_p set, TB_p terms);
TFormula_p TFormulaArithNormalize(TB_p term, TFormula_p form);
TFormula_p TFormulaArithFCodeAlloc(TB_p bank, 
      FunCode op, Type_p FunType, TFormula_p arg1, TFormula_p arg2);
TFormula_p TFormulaUnivFCodeAlloc(TB_p bank, 
      FunCode op, Type_p FunType, TFormula_p *args);

void PrintTermsDebug(TFormula_p form, int depth);

TFormula_p ACNormalizeHead(TFormula_p acterm, TB_p bank);
void ClauseNormalizeAC(Clause_p clause, TB_p bank);
ACNorm_p ACNormalize(TFormula_p acterm, TB_p bank);
void collect_ac_leafes(TFormula_p acterm, TB_p bank, FunCode rootcode, ACStruct_p head);

void ACCellAppend(ACNorm_p head, ACNorm_p tail);
ACNorm_p AllocNormalizeCell(TFormula_p leaf, bool isground);
ACStruct_p AllocNormalizeStruct();

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
