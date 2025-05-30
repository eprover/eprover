/*-----------------------------------------------------------------------

  File  : che_hcb.h

  Author: Stephan Schulz

  Contents

  Heuristic control blocks, describing heuristics for clause
  selection.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Jun  5 22:25:02 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CHE_HCB

#define CHE_HCB

#include <clb_dstacks.h>
#include <clb_permastrings.h>
#include <ccl_paramod.h>
#include <ccl_splitting.h>
#include <ccl_condensation.h>
#include <ccl_satinterface.h>
#include <ccl_unfold_defs.h>
#include <che_to_weightgen.h>
#include <che_to_precgen.h>
#include <ccl_clausefunc.h>
#include <che_wfcbadmin.h>
#include <che_litselection.h>
#include <che_to_params.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Possible ways to handle AC */

typedef enum
{
   NoACHandling,
   ACDiscardAll,
   ACKeepUnits,
   ACKeepOrientable
}ACHandlingType;

typedef enum
{
   AllLits,
   MaxLits,
   NoLits
}ExtInferenceType;

typedef enum
{
  NegMode,
  AndMode,
  OrMode,
  EqMode,
  PragmaticMode,
  FullMode,
  LogSymbolMode
} PrimEnumMode;

typedef enum
{
  SingleUnif, // use only unification which returns a single unifier
  MultiUnif // use unification which possibly returns a multiple unifiers
} UnifMode;

#define EIT2STR(x) (((x) == AllLits) ? ("all") : (((x) == MaxLits) ? "max" : "off"))
#define PEM2STR(x) ((x) == NegMode ? ("neg")\
                    : (x) == AndMode ? ("and")\
                    : (x) == OrMode ? ("or")\
                    : (x) == EqMode ? ("eq")\
                    : (x) == PragmaticMode ? ("pragmatic")\
                    : (x) == FullMode ? ("full") \
                    : (x) == LogSymbolMode ? ("logsymbol") \
                    : "unknown")
#define STR2PEM(val) (!strcmp((val), "neg") ? NegMode\
                      : !strcmp(val, "and") ? AndMode\
                      : !strcmp(val, "or") ? OrMode\
                      : !strcmp(val, "eq") ? EqMode\
                      : !strcmp(val, "pragmatic") ? PragmaticMode\
                      : !strcmp(val, "full") ? FullMode\
                      : !strcmp(val, "logsymbol") ? LogSymbolMode\
                      : (-1))
#define UM2STR(x) ((x) == SingleUnif ? "single" : "multi")
#define STR2UM(val) (!strcmp((val), "single") ? SingleUnif :\
                     !strcmp((val), "multi") ? MultiUnif : (-1))
#define NO_EXT_SUP (-1)
#define NO_ELIM_LEIBNIZ (-1)


/* External parameters for heuristics and proof control. When this is
 * changed, remember to also adapt the Init, Print und
 * Parse functions below. */

typedef struct heuristic_parms_cell
{
   /* Ordering elements */
   OrderParmsCell      order_params;

   /* Preprocessing */
   bool                no_preproc;
   long                eqdef_maxclauses;
   long                eqdef_incrlimit;
   long                formula_def_limit;
   long                miniscope_limit;

   char                *sine;
   bool                add_goal_defs_pos;
   bool                add_goal_defs_neg; /* Twee-style goal rewriting */
   bool                add_goal_defs_subterms;

   bool                bce;
   int                 bce_max_occs;

   bool                pred_elim;
   bool                pred_elim_gates;
   int                 pred_elim_max_occs;
   int                 pred_elim_tolerance;
   bool                pred_elim_force_mu_decrease;
   bool                pred_elim_ignore_conj_syms;

/* Clause selection elements */
   char                *heuristic_name;
   char                *heuristic_def;
   bool                prefer_initial_clauses;

   /* Elements controling literal selection */
   LiteralSelectionFun selection_strategy;
   long                pos_lit_sel_min;
   long                pos_lit_sel_max;
   long                neg_lit_sel_min;
   long                neg_lit_sel_max;
   long                all_lit_sel_min;
   long                all_lit_sel_max;
   long                weight_sel_min;
   bool                select_on_proc_only;
   bool                inherit_paramod_lit;
   bool                inherit_goal_pm_lit;
   bool                inherit_conj_pm_lit;

   /* Inference control elements */
   bool                enable_eq_factoring; /* Default is on! */
   bool                enable_neg_unit_paramod; /* Default is on */
   bool                enable_given_forward_simpl; /* On */

   ParamodulationType  pm_type;  /* Default is ParamodPlain */

   ACHandlingType      ac_handling;
   bool                ac_res_aggressive;

   bool                forward_context_sr;
   bool                forward_context_sr_aggressive;
   bool                backward_context_sr;

   bool                forward_subsumption_aggressive;

   RewriteLevel        forward_demod;
   bool                prefer_general;
   bool                lambda_demod;

   bool                condensing;
   bool                condensing_aggressive;

   bool                er_varlit_destructive;
   bool                er_strong_destructive;
   bool                er_aggressive;

   SplitClassType      split_clauses;
   SplitType           split_method;
   bool                split_aggressive;
   bool                split_fresh_defs;

   long                diseq_decomposition;
   long                diseq_decomp_maxarity;

   /* Global indexing */
   char                rw_bw_index_type[MAX_PM_INDEX_NAME_LEN];
   char                pm_from_index_type[MAX_PM_INDEX_NAME_LEN];
   char                pm_into_index_type[MAX_PM_INDEX_NAME_LEN];

   /* SAT checking */
   GroundingStrategy   sat_check_grounding;
   long                sat_check_step_limit;
   long                sat_check_size_limit;
   long                sat_check_ttinsert_limit;
   bool                sat_check_normconst;
   bool                sat_check_normalize;
   int                 sat_check_decision_limit;

   /* Various things */
   long                filter_orphans_limit;
   long                forward_contract_limit;
   long long           delete_bad_limit;
   rlim_t              mem_limit;
   bool                watchlist_simplify;
   bool                watchlist_is_static;
   bool                use_tptp_sos;
   bool                presat_interreduction;

   bool                detsort_bw_rw;
   bool                detsort_tmpset;

   /* Higher-order settings */
   ExtInferenceType    arg_cong;
   ExtInferenceType    neg_ext;
   ExtInferenceType    pos_ext;
   int                 ext_rules_max_depth;
   bool                inverse_recognition;
   bool                replace_inj_defs;
   bool                lift_lambdas;
   bool                lambda_to_forall;
   bool                unroll_only_formulas;
   int                 elim_leibniz_max_depth;
   PrimEnumMode        prim_enum_mode;
   int                 prim_enum_max_depth;
   int                 inst_choice_max_depth;
   bool                local_rw;
   bool                prune_args;
   bool                preinstantiate_induction;
   bool                fool_unroll;
   // unification limits
   int                 func_proj_limit;
   int                 imit_limit;
   int                 ident_limit;
   int                 elim_limit;
   // unification options
   UnifMode            unif_mode;
   bool                pattern_oracle;
   bool                fixpoint_oracle;
   int                 max_unifiers;
   int                 max_unif_steps;
}HeuristicParmsCell, *HeuristicParms_p;



/* An HCBCell describes a heuristic for clause selection. There are
   two main elemenats: A list of clause evaluation functions
   (described by a WFCB-List and a clause selection function (whose
   data is kept in the HBCCell). */

typedef struct hcb_cell
{
   /* List and number of weight function used by the heuristic. Take
      care: The list of WFCBs is ordered in the opposite direction
      from the evaluation in a clause, i.e. the _last_ WCFB will
      create the _first_ evaluation. */
   PDArray_p       wfcb_list;
   int             wfcb_no;

   /* Evaluation currently used for selection. This refers to the
      order of evaluations in the clause. See above!       */
   int             current_eval;

   /* Switching for HCBStandardClauseSelect and possibly other
      selection functions: Whenever select_count modulo
      select_switch[wfcb_no-1] reaches select_switch[current_eval],
      current_eval is increased modulo wfcb_no. */
   PDArray_p       select_switch;
   long            select_count;

   /* Selection function, this function is called to select an
      unprocessed clause from the set */

   Clause_p        (*hcb_select)(struct hcb_cell* hcb, ClauseSet_p
                                 set);

   /* Some HCB selection or evaluation functions may need data of
      their own. If yes, their creation function can allocate data,
      and needs to register a cleanup-function here. This function is
      only called if data != NULL. */
   GenericExitFun  hcb_exit;
   void*           data;
}HCBCell, *HCB_p;

#define HCB_DEFAULT_HEURISTIC "Default"

/* Default symbol occurrences limit for BCE and PE */
#define DEFAULT_SYM_OCCS 512

#define DEFAULT_MINISCOPE_LIMIT 1048576

#define DEFAULT_FILTER_ORPHANS_LIMIT LONG_MAX
#define DEFAULT_FORWARD_CONTRACT_LIMIT LONG_MAX
#define DEFAULT_DELETE_BAD_LIMIT LLONG_MAX

#define DEFAULT_RW_BW_INDEX_NAME "FP7"
#define DEFAULT_PM_FROM_INDEX_NAME "FP7"
#define DEFAULT_PM_INTO_INDEX_NAME "FP7"

typedef Clause_p (*ClauseSelectFun)(HCB_p hcb, ClauseSet_p set);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PERF_CTR_DECL(ClauseEvalTimer);

#define HeuristicParmsCellAlloc()                               \
   (HeuristicParmsCell*)SizeMalloc(sizeof(HeuristicParmsCell))
#define HeuristicParmsCellFree(junk)            \
   SizeFree(junk, sizeof(HeuristicParmsCell))

void             HeuristicParmsInitialize(HeuristicParms_p handle);
HeuristicParms_p HeuristicParmsAlloc(void);
void             HeuristicParmsFree(HeuristicParms_p junk);

void             HeuristicParmsPrint(FILE* out, HeuristicParms_p handle);
bool             HeuristicParmsParseInto(Scanner_p in, HeuristicParms_p handle,
                                         bool warn_missing);
HeuristicParms_p HeuristicParmsParse(Scanner_p in, bool warn_missing);


#define HCBCellAlloc() (HCBCell*)SizeMalloc(sizeof(HCBCell))
#define HCBCellFree(junk)        SizeFree(junk, sizeof(HCBCell))

HCB_p    HCBAlloc(void);
void     HCBFree(HCB_p junk);
long     HCBAddWFCB(HCB_p hcb, WFCB_p wfcb, long steps);
void     HCBClauseEvaluate(HCB_p hcb, Clause_p clause);
Clause_p HCBStandardClauseSelect(HCB_p hcb, ClauseSet_p set);
Clause_p HCBSingleWeightClauseSelect(HCB_p hcb, ClauseSet_p set);

long     HCBClauseSetDelProp(HCB_p hcb, ClauseSet_p set, long number,
                             FormulaProperties prop);
long HCBClauseSetDeleteBadClauses(HCB_p hcb, ClauseSet_p set, long
                                  number);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
