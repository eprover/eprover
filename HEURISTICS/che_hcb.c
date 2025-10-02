/*-----------------------------------------------------------------------

  File  : che_hcb.c

  Author: Stephan Schulz

  Contents

  Functions for the administration of HCBs.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Oct 16 14:52:53 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_hcb.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

PERF_CTR_DEFINE(ClauseEvalTimer);


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: str2eit()
//
//    Parse the value of ExtInferenceType parameter.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

ExtInferenceType str2eit(char* value)
{
   if(!strcmp(value, "all"))
   {
      return AllLits;
   }
   if(!strcmp(value, "max"))
   {
      return MaxLits;
   }
   if(!strcmp(value, "off"))
   {
      return NoLits;
   }
   Error("Unknown ExtInferenceType identifier. Expected all, max or off.", USAGE_ERROR);
   return NoLits; //will not return, stiffles compiler warning
}

/*-----------------------------------------------------------------------
//
// Function: str2eit()
//
//    Parse the value of PrimEnumMode parameter.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

PrimEnumMode str2pem(char* value)
{
   PrimEnumMode res = STR2PEM(value);
   if(res == -1)
   {
      Error("Unknown PrimEnumMode identifier.", USAGE_ERROR);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: str2eit()
//
//    Parse the value of UnifMode parameter.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

UnifMode str2um(char* value)
{
   UnifMode res = STR2UM(value);
   if(res == -1)
   {
      Error("Unknown UnifMode identifier.", USAGE_ERROR);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: get_next_clause()
//
//   Return the next clause from the selected EvalTreeTraverse-Stack,
//   or NULL if the stack is empty.
//
// Global Variables: -
//
// Side Effects    : Changes stack
//
/----------------------------------------------------------------------*/

static Clause_p get_next_clause(PStack_p *stacks, int pos)
{
   Eval_p current;

   current = EvalTreeTraverseNext(stacks[pos], pos);
   if(current)
   {
      return current->object;
   }
   return NULL;
}


void default_exit_fun(void* data)
{
   return;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsInitialize()
//
//   Initialize a heuristic parameters cell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void HeuristicParmsInitialize(HeuristicParms_p handle)
{
   OrderParmsInitialize(&(handle->order_params));

   handle->no_preproc                    = false;
   handle->eqdef_maxclauses              = DEFAULT_EQDEF_MAXCLAUSES;
   handle->eqdef_incrlimit               = DEFAULT_EQDEF_INCRLIMIT;
   handle->formula_def_limit             = 24;
   handle->miniscope_limit               = DEFAULT_MINISCOPE_LIMIT;
   handle->sine                          = NULL;

   handle->add_goal_defs_pos             = false;
   handle->add_goal_defs_neg             = false;
   handle->add_goal_defs_subterms        = false;

   handle->heuristic_name                = HCB_DEFAULT_HEURISTIC;
   handle->heuristic_def                 = NULL;
   handle->prefer_initial_clauses        = false;

   handle->bce                           = false;
   handle->bce_max_occs                  = DEFAULT_SYM_OCCS;

   handle->pred_elim                     = false;
   handle->pred_elim_gates               = false;
   handle->pred_elim_max_occs            = DEFAULT_SYM_OCCS;
   handle->pred_elim_tolerance           = 0;
   handle->pred_elim_force_mu_decrease   = false;
   handle->pred_elim_ignore_conj_syms    = false;

   handle->selection_strategy            = SelectNoLiterals;
   handle->pos_lit_sel_min               = 0;
   handle->pos_lit_sel_max               = LONG_MAX;
   handle->neg_lit_sel_min               = 0;
   handle->neg_lit_sel_max               = LONG_MAX;
   handle->all_lit_sel_min               = 0;
   handle->all_lit_sel_max               = LONG_MAX;
   handle->weight_sel_min                = 0;
   handle->select_on_proc_only           = false;
   handle->inherit_paramod_lit           = false;
   handle->inherit_goal_pm_lit           = false;
   handle->inherit_conj_pm_lit           = false;

   handle->enable_eq_factoring           = true;
   handle->enable_neg_unit_paramod       = true;
   handle->enable_given_forward_simpl    = true;
   handle->pm_type                       = ParamodPlain;

   handle->ac_handling                   = ACDiscardAll;
   handle->ac_res_aggressive             = true;

   handle->forward_context_sr            = false;
   handle->forward_context_sr_aggressive = false;
   handle->backward_context_sr           = false;

   handle->forward_subsumption_aggressive = false;

   handle->forward_demod                 = FullRewrite;
   handle->prefer_general                = false;
   handle->lambda_demod                  = false;

   handle->er_varlit_destructive         = false;
   handle->er_strong_destructive         = false;
   handle->er_aggressive                 = false;

   handle->condensing                    = false;
   handle->condensing_aggressive         = false;

   handle->split_clauses                 = SplitNone;
   handle->split_method                  = SplitGroundNone;
   handle->split_aggressive              = false;
   handle->split_fresh_defs              = true;

   handle->diseq_decomposition           = 0;
   handle->diseq_decomp_maxarity         = LONG_MAX;

   strcpy(handle->rw_bw_index_type, DEFAULT_RW_BW_INDEX_NAME);
   strcpy(handle->pm_from_index_type, DEFAULT_PM_FROM_INDEX_NAME);
   strcpy(handle->pm_into_index_type, DEFAULT_PM_INTO_INDEX_NAME);

   handle->sat_check_grounding           = GMNoGrounding;
   handle->sat_check_step_limit          = LONG_MAX;
   handle->sat_check_size_limit          = LONG_MAX;
   handle->sat_check_ttinsert_limit      = LONG_MAX;
   handle->sat_check_normconst           = false;
   handle->sat_check_normalize           = false;
   handle->sat_check_decision_limit      = 10000;
   handle->filter_orphans_limit          = DEFAULT_FILTER_ORPHANS_LIMIT;
   handle->forward_contract_limit        = DEFAULT_FORWARD_CONTRACT_LIMIT;
   handle->delete_bad_limit              = DEFAULT_DELETE_BAD_LIMIT;
   handle->mem_limit                     = 0;
   handle->watchlist_simplify            = true;
   handle->watchlist_is_static           = false;
   handle->use_tptp_sos                  = false;
   handle->presat_interreduction         = false;
   handle->detsort_bw_rw                 = false;
   handle->detsort_tmpset                = false;

   handle->arg_cong                      = AllLits;
   handle->neg_ext                       = NoLits;
   handle->pos_ext                       = NoLits;
   handle->inverse_recognition           = false;
   handle->replace_inj_defs              = false;
   handle->lift_lambdas                  = true;
   handle->ext_rules_max_depth           = NO_EXT_SUP;
   handle->lambda_to_forall              = true;
   handle->unroll_only_formulas          = true;
   handle->elim_leibniz_max_depth        = NO_ELIM_LEIBNIZ;
   handle->prim_enum_mode                = PragmaticMode;
   handle->prim_enum_max_depth           = -1;
   handle->inst_choice_max_depth         = -1;
   handle->local_rw                      = false;
   handle->prune_args                    = false;
   handle->preinstantiate_induction      = false;
   handle->fool_unroll                   = true;

   handle->func_proj_limit = 0;
   handle->imit_limit = 0;
   handle->ident_limit = 0;
   handle->elim_limit = 0;

   handle->unif_mode = SingleUnif;
   handle->pattern_oracle = true;
   handle->fixpoint_oracle = true;
   handle->max_unifiers = 4;
   handle->max_unif_steps = 256;
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsAlloc()
//
//   Allocate a cell for parameters, with initialized empty stacks.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

HeuristicParms_p HeuristicParmsAlloc(void)
{
   HeuristicParms_p handle = HeuristicParmsCellAlloc();

   HeuristicParmsInitialize(handle);
   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsFree()
//
//   Free a parameter cell.
//
// Global Variables: -
//
// Side Effects    : Memory Operations
//
/----------------------------------------------------------------------*/

void HeuristicParmsFree(HeuristicParms_p junk)
{
   assert(junk);

   HeuristicParmsCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsPrint()
//
//   Print a HeuristicParmsCell in human/computer-readable form.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void HeuristicParmsPrint(FILE* out, HeuristicParms_p handle)
{
   fprintf(out, "{\n");

   OrderParmsPrint(out, &(handle->order_params));

   fprintf(out, "   no_preproc:                     %s\n", BOOL2STR(handle->no_preproc));
   fprintf(out, "   eqdef_maxclauses:               %ld\n", handle->eqdef_maxclauses);
   fprintf(out, "   eqdef_incrlimit:                %ld\n", handle->eqdef_incrlimit);
   fprintf(out, "   formula_def_limit:              %ld\n", handle->formula_def_limit);
   fprintf(out, "   miniscope_limit:                %ld\n", handle->miniscope_limit);

   fprintf(out, "   sine:                           \"%s\"\n", handle->sine ? handle->sine : "None");
   fprintf(out, "   add_goal_defs_pos:             %s\n", BOOL2STR(handle->add_goal_defs_pos));
   fprintf(out, "   add_goal_defs_neg:             %s\n", BOOL2STR(handle->add_goal_defs_neg));
   fprintf(out, "   add_goal_defs_subterms:        %s\n", BOOL2STR(handle->add_goal_defs_subterms));

   // TEMPORARILY DISABLING BCE PRINTING AND PARSING
//    fprintf(out, "   bce:                           %s\n", BOOL2STR(handle->bce));
//    fprintf(out, "   bce_max_occs:                  %d\n", handle->bce_max_occs);

//    fprintf(out, "   pred_elim:                     %s\n", BOOL2STR(handle->pred_elim));
//    fprintf(out, "   pred_elim_gates:               %s\n", BOOL2STR(handle->pred_elim_gates));
//    fprintf(out, "   pred_elim_max_occs:            %d\n", handle->pred_elim_max_occs);
//    fprintf(out, "   pred_elim_tolerance:           %d\n", handle->pred_elim_tolerance);
//    fprintf(out, "   pred_elim_force_mu_decrease    %s\n", BOOL2STR(handle->pred_elim_force_mu_decrease));
//    fprintf(out, "   pred_elim_ignore_conj_syms     %s\n", BOOL2STR(handle->pred_elim_ignore_conj_syms));

   fprintf(out, "   heuristic_name:                %s\n", handle->heuristic_name);
   fprintf(out, "   heuristic_def:                 \"%s\"\n",
           handle->heuristic_def?handle->heuristic_def:"");
   fprintf(out, "   prefer_initial_clauses:         %s\n",
           BOOL2STR(handle->prefer_initial_clauses));

   fprintf(out, "   selection_strategy:             %s\n",
           GetLitSelName(handle->selection_strategy));

   fprintf(out, "   pos_lit_sel_min:                %ld\n", handle->pos_lit_sel_min);
   fprintf(out, "   pos_lit_sel_max:                %ld\n", handle->pos_lit_sel_max);
   fprintf(out, "   neg_lit_sel_min:                %ld\n", handle->neg_lit_sel_min);
   fprintf(out, "   neg_lit_sel_max:                %ld\n", handle->neg_lit_sel_max);
   fprintf(out, "   all_lit_sel_min:                %ld\n", handle->all_lit_sel_min);
   fprintf(out, "   all_lit_sel_max:                %ld\n", handle->all_lit_sel_max);
   fprintf(out, "   weight_sel_min:                 %ld\n", handle->weight_sel_min);

   fprintf(out, "   select_on_proc_only:            %s\n",
           BOOL2STR(handle->select_on_proc_only));
   fprintf(out, "   inherit_paramod_lit:            %s\n",
           BOOL2STR(handle->inherit_paramod_lit));
   fprintf(out, "   inherit_goal_pm_lit:            %s\n",
           BOOL2STR(handle->inherit_goal_pm_lit));
   fprintf(out, "   inherit_conj_pm_lit:            %s\n",
           BOOL2STR(handle->inherit_conj_pm_lit));

   fprintf(out, "   enable_eq_factoring:            %s\n",
           BOOL2STR(handle->enable_eq_factoring));
   fprintf(out, "   enable_neg_unit_paramod:        %s\n",
           BOOL2STR(handle->enable_neg_unit_paramod));
   fprintf(out, "   enable_given_forward_simpl:     %s\n",
           BOOL2STR(handle->enable_given_forward_simpl));

   fprintf(out, "   pm_type:                        %s\n", ParamodStr(handle->pm_type));

   fprintf(out, "   ac_handling:                    %d\n", handle->ac_handling);
   fprintf(out, "   ac_res_aggressive:              %s\n",
           BOOL2STR(handle->ac_res_aggressive));
   fprintf(out, "   forward_context_sr:             %s\n",
           BOOL2STR(handle->forward_context_sr));
   fprintf(out, "   forward_context_sr_aggressive:  %s\n",
           BOOL2STR(handle->forward_context_sr_aggressive));
   fprintf(out, "   backward_context_sr:            %s\n",
           BOOL2STR(handle->backward_context_sr));

   fprintf(out, "   forward_subsumption_aggressive: %s\n",
           BOOL2STR(handle->forward_subsumption_aggressive));

   fprintf(out, "   forward_demod:                  %d\n", handle->forward_demod);
   fprintf(out, "   prefer_general:                 %s\n",
           BOOL2STR(handle->prefer_general));
//    fprintf(out, "   lambda_demod:                 %s\n",
//            BOOL2STR(handle->lambda_demod));

   fprintf(out, "   condensing:                     %s\n",
           BOOL2STR(handle->condensing));
   fprintf(out, "   condensing_aggressive:          %s\n",
           BOOL2STR(handle->condensing_aggressive));

   fprintf(out, "   er_varlit_destructive:          %s\n",
           BOOL2STR(handle->er_varlit_destructive));
   fprintf(out, "   er_strong_destructive:          %s\n",
           BOOL2STR(handle->er_strong_destructive));
   fprintf(out, "   er_aggressive:                  %s\n",
           BOOL2STR(handle->er_aggressive));

   fprintf(out, "   split_clauses:                  %d\n", handle->split_clauses);
   fprintf(out, "   split_method:                   %d\n", handle->split_method);
   fprintf(out, "   split_aggressive:               %s\n",
           BOOL2STR(handle->split_aggressive));
   fprintf(out, "   split_fresh_defs:               %s\n",
           BOOL2STR(handle->split_fresh_defs));

   fprintf(out, "   diseq_decomposition:            %ld\n", handle->diseq_decomposition);
   fprintf(out, "   diseq_decomp_maxarity:          %ld\n", handle->diseq_decomp_maxarity);

   fprintf(out, "   rw_bw_index_type:               %s\n", handle->rw_bw_index_type);
   fprintf(out, "   pm_from_index_type:             %s\n", handle->pm_from_index_type);
   fprintf(out, "   pm_into_index_type:             %s\n", handle->pm_into_index_type);

   fprintf(out, "   sat_check_grounding:            %s\n",
           GroundingStratNames[handle->sat_check_grounding]);
   fprintf(out, "   sat_check_step_limit:           %ld\n", handle->sat_check_step_limit);
   fprintf(out, "   sat_check_size_limit:           %ld\n", handle->sat_check_size_limit);
   fprintf(out, "   sat_check_ttinsert_limit:       %ld\n",
           handle->sat_check_ttinsert_limit);
   fprintf(out, "   sat_check_normconst:            %s\n",
           BOOL2STR(handle->sat_check_normconst));
   fprintf(out, "   sat_check_normalize:            %s\n",
           BOOL2STR(handle->sat_check_normalize));
   fprintf(out, "   sat_check_decision_limit:       %d\n",
           handle->sat_check_decision_limit);

   fprintf(out, "   filter_orphans_limit:           %ld\n", handle->filter_orphans_limit);
   fprintf(out, "   forward_contract_limit:         %ld\n", handle->forward_contract_limit);
   fprintf(out, "   delete_bad_limit:               %lld\n", handle->delete_bad_limit);
   fprintf(out, "   mem_limit:                      %" PRIuMAX "\n",
           (uintmax_t)handle->mem_limit);


   fprintf(out, "   watchlist_simplify:             %s\n",
           BOOL2STR(handle->watchlist_simplify));
   fprintf(out, "   watchlist_is_static:            %s\n",
           BOOL2STR(handle->watchlist_is_static));
   fprintf(out, "   use_tptp_sos:                   %s\n",
           BOOL2STR(handle->use_tptp_sos));
   fprintf(out, "   presat_interreduction:          %s\n",
           BOOL2STR(handle->presat_interreduction));
   fprintf(out, "   detsort_bw_rw:                  %s\n",
           BOOL2STR(handle->detsort_bw_rw));
   fprintf(out, "   detsort_tmpset:                 %s\n",
           BOOL2STR(handle->detsort_tmpset));

   fprintf(out, "   arg_cong:                       %s\n",
           EIT2STR(handle->arg_cong));
   fprintf(out, "   neg_ext:                        %s\n",
           EIT2STR(handle->neg_ext));
   fprintf(out, "   pos_ext:                        %s\n",
           EIT2STR(handle->pos_ext));

   fprintf(out, "   ext_rules_max_depth:            %d\n",
           handle->ext_rules_max_depth);

   fprintf(out, "   inverse_recognition:            %s\n",
           BOOL2STR(handle->inverse_recognition));
   fprintf(out, "   replace_inj_defs:               %s\n",
           BOOL2STR(handle->replace_inj_defs));
   fprintf(out, "   lift_lambdas:                  %s\n",
           BOOL2STR(handle->lift_lambdas));
   fprintf(out, "   lambda_to_forall:              %s\n",
           BOOL2STR(handle->lambda_to_forall));
   fprintf(out, "   unroll_only_formulas:          %s\n",
           BOOL2STR(handle->unroll_only_formulas));
   fprintf(out, "   elim_leibniz_max_depth:        %d\n",
           handle->elim_leibniz_max_depth);
   fprintf(out, "   prim_enum_mode:                %s\n",
           PEM2STR(handle->prim_enum_mode));
   fprintf(out, "   prim_enum_max_depth:           %d\n",
           handle->prim_enum_max_depth);
   fprintf(out, "   inst_choice_max_depth:         %d\n",
           handle->inst_choice_max_depth);
   fprintf(out, "   local_rw:                      %s\n",
           BOOL2STR(handle->local_rw));
   fprintf(out, "   prune_args:                    %s\n",
           BOOL2STR(handle->prune_args));
   fprintf(out, "   preinstantiate_induction:      %s\n",
           BOOL2STR(handle->preinstantiate_induction));
   fprintf(out, "   fool_unroll:                   %s\n",
           BOOL2STR(handle->fool_unroll));
   fprintf(out, "   func_proj_limit:               %d\n",
           handle->func_proj_limit);
   fprintf(out, "   imit_limit:                    %d\n",
           handle->imit_limit);
   fprintf(out, "   ident_limit:                   %d\n",
           handle->ident_limit);
   fprintf(out, "   elim_limit:                    %d\n",
           handle->elim_limit);
   fprintf(out, "   unif_mode:                     %s\n",
           UM2STR(handle->unif_mode));
   fprintf(out, "   pattern_oracle:                %s\n",
           BOOL2STR(handle->pattern_oracle));
   fprintf(out, "   fixpoint_oracle:               %s\n",
           BOOL2STR(handle->fixpoint_oracle));
   fprintf(out, "   max_unifiers:                  %d\n",
           handle->max_unifiers);
   fprintf(out, "   max_unif_steps:                %d\n",
           handle->max_unif_steps);

   fprintf(out, "}\n");
}



/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsParseInto()
//
//   Parse the HeuristicParmsCell into/over the existing
//   cell. Parameters are expected in-order, but may be
//   missing. Returns true if all parameters have been found, false
//   otherwise.
//
//   The PARSE_-macros are in che_to_params.h (because they are also
//   used to parse the ordering parameters).
//
// Global Variables: -
//
// Side Effects    : Read input, may print warnings.
//
/----------------------------------------------------------------------*/

bool HeuristicParmsParseInto(Scanner_p in,
                             HeuristicParms_p handle,
                             bool warn_missing)
{
   bool res = true;

   AcceptInpTok(in, OpenCurly);

   if(TestInpTok(in, OpenCurly))
   {
      res = OrderParmsParseInto(in, &(handle->order_params), warn_missing);
   }
   else if(warn_missing)
   {
      Warning("Config misses %s\n", "ordering information");
   }

   PARSE_BOOL(no_preproc);
   PARSE_INT(eqdef_maxclauses);
   PARSE_INT(eqdef_incrlimit);

   PARSE_INT(formula_def_limit);
   PARSE_INT(miniscope_limit);
   PARSE_STRING(sine);

   PARSE_BOOL(add_goal_defs_pos);
   PARSE_BOOL(add_goal_defs_neg);
   PARSE_BOOL(add_goal_defs_subterms);

   // temporarily ignoring BCE AND PE SETTINGS.
//    PARSE_BOOL(bce);
//    PARSE_INT(bce_max_occs);

//    PARSE_BOOL(pred_elim);
//    PARSE_BOOL(pred_elim_gates);
//    PARSE_INT(pred_elim_max_occs);
//    PARSE_INT(pred_elim_tolerance);
//    PARSE_BOOL(pred_elim_force_mu_decrease);
//    PARSE_BOOL(pred_elim_ignore_conj_syms);

   PARSE_IDENTIFIER(heuristic_name);
   PARSE_STRING(heuristic_def);
   PARSE_BOOL(prefer_initial_clauses);

   if(TestInpId(in, "selection_strategy"))
   {
      NextToken(in);
      AcceptInpTok(in, Colon);
      CheckInpTok(in, Identifier);

      handle->selection_strategy = GetLitSelFun(DStrView(AktToken(in)->literal));
      if(!handle->selection_strategy)
      {
         DStr_p err = DStrAlloc();
         DStrAppendStr(err, "One of ");
         LitSelAppendNames(err);
         DStrAppendStr(err, " expected");
         AktTokenError(in, DStrView(err), false);
         DStrFree(err);
      }
      NextToken(in);
   }
   else
   {
      res = false;
      if(warn_missing)
      {
         Warning("Config misses %s\n", "selection_strategy");
      }
   }
   PARSE_INT(pos_lit_sel_min);
   PARSE_INT(pos_lit_sel_max);
   PARSE_INT(neg_lit_sel_min);
   PARSE_INT(neg_lit_sel_max);
   PARSE_INT(all_lit_sel_min);
   PARSE_INT(all_lit_sel_max);
   PARSE_INT(weight_sel_min);

   PARSE_BOOL(select_on_proc_only);
   PARSE_BOOL(inherit_paramod_lit);
   PARSE_BOOL(inherit_goal_pm_lit);
   PARSE_BOOL(inherit_conj_pm_lit);
   PARSE_BOOL(enable_eq_factoring);
   PARSE_BOOL(enable_neg_unit_paramod);
   PARSE_BOOL(enable_given_forward_simpl);

   if(TestInpId(in, "pm_type"))
   {
      NextToken(in);
      AcceptInpTok(in, Colon);
      CheckInpTok(in, Identifier);

      handle->pm_type = ParamodType(DStrView(AktToken(in)->literal));
      if(handle->pm_type == -1)
      {
         AktTokenError(in, "Proper paramod-specifier expected", false);
      }
      NextToken(in);
   }
   else
   {
      res = false;
      if(warn_missing)
      {
         Warning("Config misses %s\n", "pm_type");
      }
   }
   PARSE_INT(ac_handling);
   PARSE_BOOL(ac_res_aggressive);
   PARSE_BOOL(forward_context_sr);
   PARSE_BOOL(forward_context_sr_aggressive);
   PARSE_BOOL(backward_context_sr);
   PARSE_BOOL(forward_subsumption_aggressive);
   PARSE_INT_LIMITED(forward_demod,0,2);
   PARSE_BOOL(prefer_general);
   //PARSE_BOOL(lambda_demod);
   PARSE_BOOL(condensing);
   PARSE_BOOL(condensing_aggressive);
   PARSE_BOOL(er_varlit_destructive);
   PARSE_BOOL(er_strong_destructive);
   PARSE_BOOL(er_aggressive);
   PARSE_INT(split_clauses);
   PARSE_INT_LIMITED(split_method, 0, 2);
   PARSE_BOOL(split_aggressive);
   PARSE_BOOL(split_fresh_defs);
   PARSE_INT(diseq_decomposition);
   PARSE_INT(diseq_decomp_maxarity);

   PARSE_IDENT_INTO(rw_bw_index_type, MAX_PM_INDEX_NAME_LEN);
   PARSE_IDENT_INTO(pm_from_index_type, MAX_PM_INDEX_NAME_LEN);
   PARSE_IDENT_INTO(pm_into_index_type, MAX_PM_INDEX_NAME_LEN);

   PARSE_IDENT_NO(sat_check_grounding, GroundingStratNames);
   PARSE_INT(sat_check_step_limit);
   PARSE_INT(sat_check_size_limit);
   PARSE_INT(sat_check_ttinsert_limit);
   PARSE_BOOL(sat_check_normconst);
   PARSE_BOOL(sat_check_normalize);
   PARSE_INT(sat_check_decision_limit);

   PARSE_INT(filter_orphans_limit);
   PARSE_INT(forward_contract_limit);
   PARSE_INT(delete_bad_limit);
   PARSE_INTMAX(mem_limit);

   PARSE_BOOL(watchlist_simplify);
   PARSE_BOOL(watchlist_is_static);
   PARSE_BOOL(use_tptp_sos);
   PARSE_BOOL(presat_interreduction);
   PARSE_BOOL(detsort_bw_rw);
   PARSE_BOOL(detsort_tmpset);


   PARSE_STRING_AND_CONVERT(arg_cong, str2eit);
   PARSE_STRING_AND_CONVERT(neg_ext, str2eit);
   PARSE_STRING_AND_CONVERT(pos_ext, str2eit);
   PARSE_INT(ext_rules_max_depth);
   PARSE_BOOL(inverse_recognition);
   PARSE_BOOL(replace_inj_defs);
   PARSE_BOOL(lift_lambdas);
   PARSE_BOOL(lambda_to_forall);
   PARSE_BOOL(unroll_only_formulas);
   PARSE_INT(elim_leibniz_max_depth);
   PARSE_STRING_AND_CONVERT(prim_enum_mode, str2pem);
   PARSE_INT(prim_enum_max_depth);
   PARSE_INT(inst_choice_max_depth);
   PARSE_BOOL(local_rw);
   PARSE_BOOL(prune_args);
   PARSE_BOOL(preinstantiate_induction);
   PARSE_BOOL(fool_unroll);

   PARSE_INT(func_proj_limit);
   PARSE_INT(imit_limit);
   PARSE_INT(ident_limit);
   PARSE_INT(elim_limit);

   PARSE_STRING_AND_CONVERT(unif_mode, str2um);
   PARSE_BOOL(pattern_oracle);
   PARSE_BOOL(fixpoint_oracle);
   PARSE_INT(max_unifiers);
   PARSE_INT(max_unif_steps);


   AcceptInpTok(in, CloseCurly);
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsParse()
//
//   Parse a (newly allocated) HeuristicParmsCell and return it.
//
// Global Variables: -
//
// Side Effects    : Memory, Input
//
/----------------------------------------------------------------------*/

HeuristicParms_p HeuristicParmsParse(Scanner_p in, bool warn_missing)
{
   HeuristicParms_p res = HeuristicParmsAlloc();
   HeuristicParmsParseInto(in, res, warn_missing);
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: HCBAlloc()
//
//   Return an empty, initialized HCB.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

HCB_p HCBAlloc(void)
{
   HCB_p handle = HCBCellAlloc();

   handle->wfcb_list     = PDArrayAlloc(4,4);
   handle->wfcb_no       = 0;
   handle->current_eval  = 0;
   handle->select_switch = PDArrayAlloc(4,4);
   handle->select_count  = 0;
   handle->hcb_select    = HCBStandardClauseSelect;
   handle->hcb_exit      = default_exit_fun;
   handle->data          = NULL;

   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: HCBFree()
//
//   Free a heuristics control block.
//
// Global Variables: -
//
// Side Effects    : Memory operation, releases WFCB's
//
/----------------------------------------------------------------------*/

void HCBFree(HCB_p junk)
{
   assert(junk);

   /* WFCB's are _not_ freed, as they come from a WFCBAdmin-Block
      anyways! */
   PDArrayFree(junk->wfcb_list);
   PDArrayFree(junk->select_switch);
   if(junk->data)
   {
      junk->hcb_exit(junk->data);
   }
   HCBCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: HCBAddWFCB()
//
//   Add a WFCB with to the HCB, adjust selection function. Return
//   number of weight functions in HCB.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long HCBAddWFCB(HCB_p hcb, WFCB_p wfcb, long steps)
{
   assert(hcb);
   assert(wfcb);
   assert(steps >0);

   if(hcb->wfcb_no)
   {
      steps+= PDArrayElementInt(hcb->select_switch, hcb->wfcb_no-1);
   }
   PDArrayAssignP(hcb->wfcb_list, hcb->wfcb_no, wfcb);
   PDArrayAssignInt(hcb->select_switch, hcb->wfcb_no, steps);
   hcb->wfcb_no++;

   hcb->hcb_select = (hcb->wfcb_no != 1) ?
      HCBStandardClauseSelect : HCBSingleWeightClauseSelect;

   return hcb->wfcb_no;
}



/*-----------------------------------------------------------------------
//
// Function: HCBClauseEvaluate()
//
//   Giben a HCB-Block, add evaluations to the given clause.
//
// Global Variables: -
//
// Side Effects    : Memory, adds evaluations, by eval functions
//
/----------------------------------------------------------------------*/

void HCBClauseEvaluate(HCB_p hcb, Clause_p clause)
{
   long i;
   bool empty;

   PERF_CTR_ENTRY(ClauseEvalTimer);
   assert(clause->evaluations == NULL);
   ClauseAddEvalCell(clause, EvalsAlloc(hcb->wfcb_no));

   empty = ClauseIsSemFalse(clause);
   for(i=0; i< hcb->wfcb_no; i++)
   {
      ClauseAddEvaluation(PDArrayElementP(hcb->wfcb_list, i), clause, i, empty);
   }
   PERF_CTR_EXIT(ClauseEvalTimer);
}

/*-----------------------------------------------------------------------
//
// Function: HCBStandardClauseSelect()
//
//   Select a clause from set, based on the evaluations and the data
//   in hcb.
//
// Global Variables: -
//
// Side Effects    : Modifies HCB data
//
/----------------------------------------------------------------------*/

Clause_p HCBStandardClauseSelect(HCB_p hcb, ClauseSet_p set)
{
   Clause_p clause;

   clause = ClauseSetFindBest(set, hcb->current_eval);
   while(clause && ClauseIsOrphaned(clause))
   {
      ClauseSetExtractEntry(clause);
      ClauseFree(clause);
      clause = ClauseSetFindBest(set, hcb->current_eval);
   }
   hcb->select_count++;
   while(hcb->select_count ==
         PDArrayElementInt(hcb->select_switch,hcb->current_eval))
   {
      hcb->current_eval++;
   }
   if(hcb->current_eval == hcb->wfcb_no)
   {
      hcb->select_count = 0;
      hcb->current_eval = 0;
   }
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: HCBSingleWeightClauseSelect()
//
//   Select a clause from the set based on the first weight.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

Clause_p HCBSingleWeightClauseSelect(HCB_p hcb, ClauseSet_p set)
{
   Clause_p clause;

   clause = ClauseSetFindBest(set, hcb->current_eval);
   while(clause && ClauseIsOrphaned(clause))
   {
      ClauseSetExtractEntry(clause);
      ClauseFree(clause);
      clause = ClauseSetFindBest(set, hcb->current_eval);
   }
   return clause;
}


/*-----------------------------------------------------------------------
//
// Function: HCBClauseSetDelProp()
//
//   Delete the property prop from the first number clauses in set
//   that would be picked according to hcb. Note that this is _not_
//   reliable, as in real processing, clauses that would have been
//   picked may vanish due to missing parents. It should be a fairly
//   good approximation, though.
//
// Global Variables: -
//
// Side Effects    : Memory operations, changes flags.
//
/----------------------------------------------------------------------*/

long HCBClauseSetDelProp(HCB_p hcb, ClauseSet_p set, long number,
                         FormulaProperties prop)
{
   int i, j;
   long prop_cleared = 0;
   PStack_p *stacks = SizeMalloc(hcb->wfcb_no*sizeof(PStack_p));
   Clause_p clause;

   for(i=0; i< hcb->wfcb_no; i++)
   {
      stacks[i]=
         EvalTreeTraverseInit(PDArrayElementP(set->eval_indices, i),i);
   }
   while(number)
   {
      for(i=0; i < hcb->wfcb_no; i++)
      {
         for(j=0; j < PDArrayElementInt(hcb->select_switch, j); j++)
         {
            while((clause =
                   get_next_clause(stacks,i)))
            {
               if(ClauseQueryProp(clause, prop))
               {
                  ClauseDelProp(clause, prop);
                  prop_cleared++;
                  break;
               }
            }
            number--; /* We did our best - this is an easy catch for
                         the stupid case number > set->members */
            if(!number)
            {
               break;
            }
         }
         if(!number)
         {
            break;
         }
      }
   }
   for(i=0; i< hcb->wfcb_no; i++)
   {
      EvalTreeTraverseExit(stacks[i]);
   }
   SizeFree(stacks, hcb->wfcb_no*sizeof(PStack_p));

   return prop_cleared;
}


/*-----------------------------------------------------------------------
//
// Function: HCBClauseSetDeleteBadClauses()
//
//   Delete all but the best number clauses from the set.
//
// Global Variables: -
//
// Side Effects    : Changes set.
//
/----------------------------------------------------------------------*/

long HCBClauseSetDeleteBadClauses(HCB_p hcb, ClauseSet_p set, long
                                  number)
{
   long res;

   ClauseSetSetProp(set, CPDeleteClause);
   HCBClauseSetDelProp(hcb, set, number, CPDeleteClause);
   res = ClauseSetDeleteMarkedEntries(set);

   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
