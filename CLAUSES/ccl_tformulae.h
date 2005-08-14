/*-----------------------------------------------------------------------

File  : ccl_tformulae.h

Author: Stephan Schulz

Contents
 
  Declarations and definitions for full first-order formulae encoded
  as terms.

  Copyright 2003, 2005 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat May 14 08:50:45 CEST 2005
    New (translated from ccl_formulae.h)

-----------------------------------------------------------------------*/

#ifndef CCL_TFORMULAE

#define CCL_TFORMULAE

#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef Term_p TFormula_p;

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define   TFormulaHasSubForm1(sig, form)\
          (SigQueryFuncProp((sig),(form)->f_code, FPFOFOp) &&\
          ((form)->arity>=1))
#define   TFormulaHasSubForm2(sig, form) \
          (SigQueryFuncProp((sig),(form)->f_code, FPFOFOp) &&\
          ((form)->arity>=2))

#define   TFormulaIsBinary(form)     ((form)->arity==2)
#define   TFormulaIsUnary(form)      ((form)->arity==1)
#define   TFormulaIsQuantified(sig,form)\
          ((form)->f_code == sig->qex_code || (form)->f_code == sig->qall_code )
#define   TFormulaIsLiteral(sig,form)    (SigIsPredicate((sig), (form)->f_code))
#define   TFormulaIsPropConst(form, positive)\
          ((positive)?((form)->f_code == SIG_TRUE_CODE):\
                      ((form)->f_code == SIG_FALSE_CODE))

#define   TFormulaIsPropTrue(form)  ((form)->f_code == SIG_TRUE_CODE)
#define   TFormulaIsPropFalse(form) ((form)->f_code == SIG_FALSE_CODE)


#define    TFormulaGCMarkCells(bank, form) TBGCMarkTerm((bank),(form))
TFormula_p TFormulaFCodeAlloc(TB_p bank, FunCode op, TFormula_p arg1, TFormula_p arg2);
TFormula_p TFormulaLitAlloc(Eqn_p literal);
TFormula_p TFormulaPropConstantAlloc(TB_p bank, bool positive);
TFormula_p TFormulaQuantorAlloc(TB_p bank, FunCode quantor, Term_p var, TFormula_p arg);
void       TFormulaTPTPPrint(FILE* out, TB_p bank, TFormula_p form, bool fullterms, bool pcl);
TFormula_p TFormulaTPTPParse(Scanner_p in, TB_p terms);

#define    TFormulaEqual(f1,f2) ((f1)==(f2))
bool       TFormulaVarIsFree(TB_p bank, TFormula_p form, Term_p var);
#define    TFormulaCopy(bank, form) TBInsertNoProps(bank, form, DEREF_ALWAYS)

void       TFormulaCollectFreeVars(TB_p bank, TFormula_p form, PTree_p *vars);

TFormula_p TFormulaAddQuantor(TB_p bank, TFormula_p form, bool universal, Term_p var);
TFormula_p TFormulaAddQuantors(TB_p bank, TFormula_p form, bool universal,
                               PTree_p vars); 
TFormula_p TFormulaClosure(TB_p bank, TFormula_p form, bool universal);

#define   TFormulaFindMaxVarCode(form) TermFindMaxVarCode(form)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





