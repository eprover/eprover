/*-----------------------------------------------------------------------

File  : che_hcb.h

Author: Stephan Schulz

Contents

  Heuristic control blocks, describing heuristics for clause
  selection.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun  5 22:25:02 MET DST 1998
    New
<2> Wed Dec 16 23:17:21 MET 1998
    Integrate HeuristicParms stuff

-----------------------------------------------------------------------*/

#ifndef CHE_HCB

#define CHE_HCB

#include <clb_dstacks.h>
#include <ccl_paramod.h>
#include <ccl_splitting.h>
#include <ccl_condensation.h>
#include <che_to_weightgen.h>
#include <che_to_precgen.h>
#include <ccl_clausefunc.h>
#include <che_wfcbadmin.h>
#include <che_litselection.h>
#include <time.h>

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


/* External parameters for heuristics and proof control */

typedef struct heuristic_parms_cell
{
   /* Clause selection elements */
   char                *heuristic_name;
   bool                prefer_initial_clauses;

   /* Ordering elements */
   TermOrdering        ordertype;
   TOWeightGenMethod   to_weight_gen;
   TOPrecGenMethod     to_prec_gen;
   char*               to_pre_prec;
   char*               to_pre_weights;
   long                to_const_weight;
   bool                no_lit_cmp;

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

   RewriteLevel        forward_demod;
   bool                prefer_general;

   bool                condensing;
   bool                condensing_aggressive;

   bool                er_varlit_destructive;
   bool                er_strong_destructive;
   bool                er_aggressive;

   SplitClassType      split_clauses;
   SplitType           split_method;
   bool                split_aggressive;
   bool                split_fresh_defs;

   /* Global indexing */
   char                rw_bw_index_type[MAX_PM_INDEX_NAME_LEN];
   char                pm_from_index_type[MAX_PM_INDEX_NAME_LEN];
   char                pm_into_index_type[MAX_PM_INDEX_NAME_LEN];

   /* Various things */
   long                filter_limit;
   long                filter_copies_limit;
   long                reweight_limit;
   long long           delete_bad_limit;
   rlim_t              mem_limit;
   bool                watchlist_simplify;
   bool                use_tptp_sos;
   bool                presat_interreduction;

   bool                detsort_bw_rw;
   bool                detsort_tmpset;
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
   Clause_p        (*hcb_select)(struct hcb_cell* hcb, ClauseSet_p set,
                                 PQueue_p pending);

   /* Some HCB selection or evaluation functions may need data of
      their own. If yes, their creation function can allocate data,
      and needs to register a cleanup-function here. This function is
      only called if data != NULL. */
   GenericExitFun  hcb_exit;
   void*           data;

   /* True when flag to print the batch sizes is set */
   bool            save_batch_sizes;
   /* Stores the number of clauses in pending when the pending queue is
      evaluated and inserted into unprocessed. */
   PQueue_p        list_batch_sizes;
}HCBCell, *HCB_p;

#define HCB_DEFAULT_HEURISTIC "Default"
#define DEFAULT_FILTER_LIMIT LONG_MAX
#define DEFAULT_FILTER_COPIES_LIMIT LONG_MAX
#define DEFAULT_REWEIGHT_INTERVAL LONG_MAX
#define DEFAULT_DELETE_BAD_LIMIT LONG_MAX

#define DEFAULT_RW_BW_INDEX_NAME "FP7"
#define DEFAULT_PM_FROM_INDEX_NAME "FP7"
#define DEFAULT_PM_INTO_INDEX_NAME "FP7"

typedef Clause_p (*ClauseSelectFun)(HCB_p hcb, ClauseSet_p set);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

PERF_CTR_DECL(ClauseEvalTimer);

extern  int   LimitTF;
extern  int   TFEvalLimit;
extern  int   TFTimeLimit;
extern  int   TFLimitReached;
extern  int   TFNum;
extern  int   TFNumEval;
extern  struct timespec TFLimitStart;

#define HeuristicParmsCellAlloc() \
   (HeuristicParmsCell*)SizeMalloc(sizeof(HeuristicParmsCell))
#define HeuristicParmsCellFree(junk) \
   SizeFree(junk, sizeof(HeuristicParmsCell))

void             HeuristicParmsInitialize(HeuristicParms_p handle);
HeuristicParms_p HeuristicParmsAlloc(void);
void             HeuristicParmsFree(HeuristicParms_p junk);


#define HCBCellAlloc() (HCBCell*)SizeMalloc(sizeof(HCBCell))
#define HCBCellFree(junk)        SizeFree(junk, sizeof(HCBCell))

HCB_p    HCBAlloc(void);
void     HCBFree(HCB_p junk);
long     HCBAddWFCB(HCB_p hcb, WFCB_p wfcb, long steps);
long     ClauseSetEvalInsertQueue(ClauseSet_p set, PQueue_p from, HCB_p hcb,
                                  bool is_ctrl_hcb, int diff);
void     HCBClauseEvaluate(HCB_p hcb, Clause_p clause);
void     UpdateHCBEvalCounters(HCB_p hcb);
Clause_p HCBStandardClauseSelect(HCB_p hcb, ClauseSet_p set, PQueue_p pending);
Clause_p HCBSingleWeightClauseSelect(HCB_p hcb, ClauseSet_p set,
                                     PQueue_p pending);

long     HCBClauseSetDelProp(HCB_p hcb, ClauseSet_p set, long number,
                             FormulaProperties prop);
long HCBClauseSetDeleteBadClauses(HCB_p hcb, ClauseSet_p set, long
              number);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
