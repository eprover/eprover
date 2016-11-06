/*-----------------------------------------------------------------------

File  : ccl_tcnf.h

Author: Stephan Schulz

Contents

  Functions implementing (eventually) the CNF conversion of first
  order formulae encoded as terms.

  Copyright 2003,2005 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri May 20 09:45:25 CEST 2005
    New (taken some from ccl_cnf.h).

-----------------------------------------------------------------------*/

#ifndef CCL_TCNF

#define CCL_TCNF

#include <clb_numxtrees.h>
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

long       TFormulaEstimateClauses(TB_p bank, TFormula_p form, bool pos);

TFormula_p TFormulaDefRename(TB_p bank, TFormula_p form, int polarity,
                             NumXTree_p *defs, PStack_p renamed_forms);
void TFormulaFindDefs(TB_p bank, TFormula_p form, int polarity,
                      long def_limit, NumXTree_p *defs,
                      PStack_p renamed_forms);
TFormula_p TFormulaCopyDef(TB_p bank, TFormula_p form, long blocked,
                           NumXTree_p *defs, PStack_p defs_used);
TFormula_p TFormulaSimplify(TB_p terms, TFormula_p form, bool full_simpl);

TFormula_p TFormulaNNF(TB_p terms, TFormula_p form, int polarity);

TFormula_p TFormulaMiniScope(TB_p terms,  TFormula_p form);

TFormula_p TFormulaVarRename(TB_p terms, TFormula_p form);

TFormula_p TFormulaSkolemizeOutermost(TB_p terms, TFormula_p form);

TFormula_p TFormulaDistributeDisjunctions(TB_p terms, TFormula_p form);

void WTFormulaConjunctiveNF(WFormula_p form, TB_p terms);
void WTFormulaConjunctiveNF2(WFormula_p form, TB_p terms,
                             long miniscope_limit);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
