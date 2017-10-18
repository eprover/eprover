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

long WFormulaCNF(WFormula_p form, ClauseSet_p set,
                 TB_p terms, VarBank_p fresh_vars);
long WFormulaCNF2(WFormula_p form, ClauseSet_p set,
                  TB_p terms, VarBank_p fresh_vars,
                  long miniscope_limit);
long FormulaSetSimplify(FormulaSet_p set, TB_p terms);

long FormulaSetCNF(FormulaSet_p set, FormulaSet_p archive,
                   ClauseSet_p clauseset, TB_p terms,
                   VarBank_p fresh_vars, GCAdmin_p gc);
long FormulaSetCNF2(FormulaSet_p set, FormulaSet_p archive,
                    ClauseSet_p clauseset, TB_p terms,
                    VarBank_p fresh_vars, GCAdmin_p gc,
                    long miniscope_limit);
long FormulaAndClauseSetParse(Scanner_p in, ClauseSet_p cset,
                              FormulaSet_p fset, TB_p terms,
                              StrTree_p *name_selector,
                              StrTree_p *skip_includes);
long FormulaToCNF(WFormula_p form, FormulaProperties type,
                  ClauseSet_p set, TB_p terms, VarBank_p fresh_vars);
long TFormulaToCNF(WFormula_p form, FormulaProperties type, ClauseSet_p set,
                   TB_p terms, VarBank_p fresh_vars);

void TFormulaSetDelTermpProp(FormulaSet_p set, TermProperties prop);
void TFormulaSetFindDefs(FormulaSet_p set, TB_p terms, NumXTree_p *defs,
                         PStack_p renamed_forms);
long TFormulaApplyDefs(WFormula_p form, TB_p terms, NumXTree_p *defs);
long TFormulaSetIntroduceDefs(FormulaSet_p set,
                              FormulaSet_p archive,
                              TB_p terms);

void FormulaSetArchive(FormulaSet_p set, FormulaSet_p archive);
void FormulaSetDocInital(FILE* out, long level, FormulaSet_p set);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
