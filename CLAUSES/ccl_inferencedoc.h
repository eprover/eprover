/*-----------------------------------------------------------------------

File  : ccl_inferencedoc.h

Author: Stephan Schulz

Contents

  Functions and constants for reporting on the proof process.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jan  5 15:27:37 MET 1999
    Partially new, partially lifted from ccl_clauses.[ch]

-----------------------------------------------------------------------*/

#ifndef CCL_INFERENCEDOC

#define CCL_INFERENCEDOC

#include <ccl_clausepos.h>
#include <ccl_formula_wrapper.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef enum
{
   inf_noinf,
   inf_initial,
   inf_quote,
   inf_evalgc,
   inf_paramod,
   inf_sim_paramod,
   inf_efactor,
   inf_factor,
   inf_eres,
   inf_split,
   inf_simplify_reflect,
   inf_context_simplify_reflect,
   inf_ac_resolution,
   inf_condense,
   inf_minimize,
   inf_rewrite,

   inf_fof_simpl,
   inf_fof_split_equiv,
   inf_fof_nnf,
   inf_fof_intro_def,
   inf_fof_apply_def,
   inf_shift_quantors,
   inf_fof_distrib,
   inf_annotate_question,
   inf_eval_answers,
   inf_var_rename,
   inf_skolemize_out,
   inf_neg_conjecture,
   inf_unroll_fool
}InfType;

typedef enum
{
   no_format,
   lop_format,
   pcl_format,
   tstp_format,
   tptp_format,
   xml_format
}OutputFormatType;


#define PCL_QUOTE  NULL
#define PCL_EVALGC "evalgc"
#define PCL_ER     "er"
#define PCL_DDC    "diseq_decomp"
#define PCL_PM     "pm"
#define PCL_SPM    "spm"
#define PCL_EF     "ef"
#define PCL_OF     "of"
#define PCL_SAT    "cdclpropres"
#define PCL_PE_RESOLVE    "pred_elim_resolve"
#define PCL_SPLIT  "split"
#define TSTP_SPLIT_REFINED "esplit"
#define TSTP_SPLIT_BASE    "split"
#define PCL_RW     "rw"
#define PCL_LOCAL_RW "local_rw"
#define PCL_SR     "sr"
#define PCL_CSR    "csr"
#define PCL_ACRES  "ar"
#define PCL_CN     "cn"
#define PCL_CONDENSE "condense"

#define PCL_SC         "split_conjunct"
#define PCL_SE         "split_equiv"
#define PCL_FS         "fof_simplification"
#define PCL_NNF        "fof_nnf"
#define PCL_ID         "introduced"
#define PCL_ID_DEF     "introduced(definition)"
#define PCL_AD         "apply_def"
#define PCL_SQ         "shift_quantors"
#define PCL_VR         "variable_rename"
#define PCL_SK         "skolemize"
#define PCL_DSTR       "distribute"
#define PCL_EQ_TO_EQ   "lift_bool_eq"
#define PCL_LL         "lift_lambdas"
#define PCL_ANNOQ      "add_answer_literal"
#define PCL_EXPDISTICT "epxand_distinct"
#define PCL_EVANS      "eval_answer_literal"
#define PCL_NC         "assume_negation"
#define PCL_FU         "fool_unroll"
#define PCL_LIFT_ITE   "lift_ite"
#define PCL_EBV        "eliminate_boolean_vars"
#define PCL_DYN_CNF    "dynamic_cnf"
#define PCL_FLEX_RESOLVE "flex_resolve"
#define PCL_ARG_CONG "arg_cong"
#define PCL_NEG_EXT "neg_ext"
#define PCL_POS_EXT "pos_ext"
#define PCL_EXT_SUP "ext_sup"
#define PCL_EXT_EQRES "ext_eqres"
#define PCL_EXT_EQFACT "ext_eqfact"
#define PCL_INV_REC "recognize_injectivity"
#define PCL_CHOICE_AX "introduce_choice_axiom"
#define PCL_LEIBNIZ_ELIM "eliminate_leibniz_eq"
#define PCL_PRIM_ENUM "primitive_enumeration"
#define PCL_CHOICE_INST "choice_inst"
#define PCL_TRIGGER "trigger"
#define PCL_PRUNE_ARG "prune_arg"


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern OutputFormatType DocOutputFormat;
extern bool             PCLFullTerms;
extern bool             PCLStepCompact;
extern int              PCLShellLevel;

char*  PCLTypeStr(FormulaProperties type);

void    DocClauseCreation(FILE* out, long level, Clause_p clause,
                          InfType op, Clause_p parent1,
                          Clause_p parent2, char* comment);
#define DocClauseCreationDefault(clause, op, parent1, parent2)\
        DocClauseCreation(GlobalOut, OutputLevel, (clause),\
             (op), (parent1), (parent2), NULL)

void    DocClauseFromForm(FILE* out, long level, Clause_p clause,
                          WFormula_p parent);

void    DocClauseModification(FILE* out, long level, Clause_p clause, InfType
           op, Clause_p partner, Sig_p sig, char* comment);
#define DocClauseModificationDefault(clause, op, partner)\
        DocClauseModification(GlobalOut, OutputLevel, (clause), (op),\
           (partner), NULL, NULL)

void    DocClauseQuote(FILE* out, long level, long target_level,
             Clause_p clause, char* comment, Clause_p
             opt_partner);

#define DocClauseQuoteDefault(target_level, clause, comment)\
        DocClauseQuote(GlobalOut, OutputLevel, (target_level),\
             (clause), (comment), NULL)

void    DocClauseRewrite(FILE* out, long level, ClausePos_p rewritten,
          Term_p old_term, char* comment);

#define DocClauseRewriteDefault(rewritten, old_term)\
        DocClauseRewrite(GlobalOut, OutputLevel, (rewritten),\
          (old_term), NULL);
void    DocClauseEqUnfold(FILE* out, long level, Clause_p rewritten,
           ClausePos_p demod, PStack_p demod_pos);


void    DocFormulaCreation(FILE* out, long level, WFormula_p formula,
                           InfType op, WFormula_p parent1,
                           WFormula_p parent2, char* comment);

#define DocFormulaCreationDefault(formula, op, parent1, parent2)\
        DocFormulaCreation(GlobalOut, OutputLevel, (formula),\
        (op), (parent1), (parent2), NULL)

void    DocFormulaModification(FILE* out, long level, WFormula_p form,
                                InfType op, char* comment);

#define DocFormulaModificationDefault(form, op)\
        DocFormulaModification(GlobalOut, OutputLevel, (form), (op), NULL)

void    DocFormulaIntroDefs(FILE* out, long level, WFormula_p form,
                            PStack_p def_list, char* comment);
#define DocFormulaIntroDefsDefault(form, def_list)\
        DocFormulaIntroDefs(GlobalOut, OutputLevel, (form), (def_list), NULL)


void    DocIntroSplitDef(FILE* out, long level, WFormula_p form);

#define DocIntroSplitDefDefault(form)\
        DocIntroSplitDef(GlobalOut, OutputLevel, (form))

void    DocIntroSplitDefRest(FILE* out, long level, Clause_p clause,
                             WFormula_p parent, char* comment);
#define DocIntroSplitDefRestDefault(clause, parent)\
        DocIntroSplitDefRest(GlobalOut, OutputLevel, (clause), (parent), NULL)

void    DocClauseApplyDefs(FILE* out, long level, Clause_p clause,
                           long parent_id, PStack_p def_ids,
                           char* comment);
#define DocClauseApplyDefsDefault(clause, parent_id, def_ids)\
        DocClauseApplyDefs(GlobalOut, OutputLevel, (clause), \
                           (parent_id), (def_ids), NULL)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
