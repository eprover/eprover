#include <ccl_formulasets.h>
#include <cte_termbanks.h>
#include <ccl_tformulae.h>
#include <cte_functypes.h>
#include <cte_simpletypes.h>
#include <ccl_derivation.h>
#include <ccl_garbage_coll.h>

void FormulaSetArithNorm(FormulaSet_p set, TB_p terms, GCAdmin_p gc);
TFormula_p TFormulaArithNormalize(TB_p term, TFormula_p form);
TFormula_p TFormulaArithFCodeAlloc(TB_p bank, 
      FunCode op, Type_p FunType, TFormula_p arg1, TFormula_p arg2);

void PrintTermsDebug(TFormula_p form, TB_p terms, int depth);
