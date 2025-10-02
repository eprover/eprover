/*-----------------------------------------------------------------------

File  : ccl_formulafunc.h

Author: Stephan Schulz

Contents

  Higher level Formula functions that need to know about sets (and
  CNFing).

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Apr  4 14:10:19 CEST 2004
    New

-----------------------------------------------------------------------*/

#ifndef CCL_FORMULAFUNC

#define CCL_FORMULAFUNC

#include <ccl_garbage_coll.h>
#include <ccl_tcnf.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define TFORMULA_GC_LIMIT 1.5

bool WFormulaConjectureNegate(WFormula_p wform);
#define WFormulaTSTPPrintDeriv(out, form) \
   WFormulaTSTPPrint((out),(form), true, false);        \
   fprintf((out), ", ");                                                \
   DerivationStackTSTPPrint((out), (form)->terms->sig, (form)->derivation); \
   fprintf(out, ").");


int  FormulaCountNonTopLevelLambdas(Sig_p sig, TFormula_p form);
bool FormulaHasAppVarLit(Sig_p sig, TFormula_p form);

TFormula_p TFormulaAnnotateQuestion(TB_p terms,
                                    TFormula_p form,
                                    NumTree_p *question_assoc);

bool WFormulaAnnotateQuestion(WFormula_p wform, bool add_answer_lits,
                              bool conjectures_are_questions,
                              NumTree_p *question_assoc);

long FormulaSetPreprocConjectures(FormulaSet_p set,
                                  FormulaSet_p archive,
                                  bool add_answer_lits,
                                  bool conjectures_are_questions);


bool WFormulaSimplify(WFormula_p form, TB_p terms);

/* long WFormulaCNF(WFormula_p form, ClauseSet_p set, */
/*                  TB_p terms, VarBank_p fresh_vars); */
long WFormulaCNF2(WFormula_p form, ClauseSet_p set,
                  TB_p terms, VarBank_p fresh_vars,
                  long miniscope_limit,
                  bool fool_unroll);
long FormulaSetSimplify(FormulaSet_p set, TB_p terms, bool gc);

/* long FormulaSetCNF(FormulaSet_p set, FormulaSet_p archive, */
/*                    ClauseSet_p clauseset, TB_p terms, */
/*                    VarBank_p fresh_vars,long def_limit); */
long FormulaSetCNF2(FormulaSet_p set, FormulaSet_p archive,
                    ClauseSet_p clauseset, TB_p terms,
                    VarBank_p fresh_vars,
                    long miniscope_limit, long def_limit,
                    bool lift_lambdas, bool lambda_to_forall, bool unfold_only_form,
                    bool unroll_fool);
long FormulaAndClauseSetParse(Scanner_p in, FormulaSet_p fset,
                              ClauseSet_p wlset, TB_p terms,
                              StrTree_p *name_selector,
                              StrTree_p *skip_includes);
long FormulaToCNF(WFormula_p form, FormulaProperties type,
                  ClauseSet_p set, TB_p terms, VarBank_p fresh_vars);
long TFormulaToCNF(WFormula_p form, FormulaProperties type, ClauseSet_p set,
                   TB_p terms, VarBank_p fresh_vars);

void TFormulaSetDelTermpProp(FormulaSet_p set, TermProperties prop);
void TFormulaSetFindDefs(FormulaSet_p set, TB_p terms, NumXTree_p *defs,
                         PStack_p renamed_forms, long limit);
long TFormulaApplyDefs(WFormula_p form, TB_p terms, NumXTree_p *defs);
long TFormulaSetIntroduceDefs(FormulaSet_p set,
                              FormulaSet_p archive,
                              TB_p terms, long limit);

void FormulaSetArchive(FormulaSet_p set, FormulaSet_p archive);
void FormulaSetDocInital(FILE* out, long level, FormulaSet_p set);

long TFormulaSetLiftItes(FormulaSet_p set, FormulaSet_p archive, TB_p terms);
long TFormulaSetLiftLets(FormulaSet_p set, FormulaSet_p archive, TB_p terms);
bool WFormulaReplaceEqnWithEquiv(WFormula_p form, TB_p terms);
long WFormulaSetUnrollFOOL(FormulaSet_p set, FormulaSet_p archive, TB_p terms);
long TFormulaSetLambdaNormalize(FormulaSet_p set, FormulaSet_p archive, TB_p terms);
long TFormulaSetNamedToDBLambdas(FormulaSet_p set, FormulaSet_p archive, TB_p terms);
long TFormulaSetUnfoldDefSymbols(FormulaSet_p set, FormulaSet_p archive, TB_p terms, bool only_forms);
bool TFormulaUnrollFOOL(WFormula_p form, TB_p terms);
long TFormulaSetLiftLambdas(FormulaSet_p set, FormulaSet_p archive, TB_p terms);
void ClauseSetLiftLambdas(ClauseSet_p set, FormulaSet_p archive, TB_p terms, VarBank_p fresh_vars, bool unroll_fool);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
