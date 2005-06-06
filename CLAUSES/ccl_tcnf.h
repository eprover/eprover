/*-----------------------------------------------------------------------

File  : ccl_tcnf.h

Author: Stephan Schulz

Contents

  Functions implementing (eventually) the CNF conversion of first
  order formulae encoded as terms. 

  Copyright 2003,2005 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri May 20 09:45:25 CEST 2005
    New (taken some from ccl_cnf.h).

-----------------------------------------------------------------------*/

#ifndef CCL_TCNF

#define CCL_TCNF

#include <ccl_tformulae.h>
#include <ccl_clausesets.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TFORM_MANY_CLAUSES LONG_MAX
#define TFORM_MANY_LIMIT   1024
#define TFORM_RENAME_LIMIT 8

long       TFormulaEstimateClauses(TB_p bank, TFormula_p form, bool pos);

TFormula_p TFormulaDefRename(TB_p bank, TFormula_p form, int polarity, 
                             NumTree_p *defs);

TFormula_p TFormulaSimplify(TB_p terms, TFormula_p form);

TFormula_p TFormulaNNF(TB_p terms, TFormula_p form, int polarity);

TFormula_p TFormulaMiniScope(TB_p terms,  TFormula_p form);

TFormula_p TFormulaVarRename(TB_p terms, TFormula_p form);

TFormula_p TFormulaSkolemizeOutermost(TB_p terms, TFormula_p form);

TFormula_p TFormulaDistributeDisjunctions(TB_p terms, TFormula_p form);

void WTFormulaConjunctiveNF(WFormula_p form, TB_p terms);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





