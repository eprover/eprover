/*-----------------------------------------------------------------------

  File  : eprover.c

  Author: Stephan Schulz

  Contents

  Main program for the E equational theorem prover.

  Copyright 1998-2024 by the authors.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jun  9 01:32:15 MET DST 1998

-----------------------------------------------------------------------*/

#include <clb_defines.h>
#include <clb_regmem.h>
#include <cio_commandline.h>
#include <cio_output.h>
#include <ccl_relevance.h>
#include <cco_proofproc.h>
#include <cco_sine.h>
#include <cio_signals.h>
#include <ccl_unfold_defs.h>
#include <ccl_formulafunc.h>
#include <cte_simpletypes.h>
#include <cco_scheduling.h>
#include <e_version.h>
#include <cte_lambda.h>
#include <cco_ho_inferences.h>
#include <che_new_autoschedule.h>
#include <cco_preprocessing.h>
#include <sys/mman.h>


/*---------------------------------------------------------------------*/
/*                  Data types                                         */
/*---------------------------------------------------------------------*/

#define NAME         "eprover"

PERF_CTR_DEFINE(SatTimer);

#include <e_options.h>


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char              *outname = NULL;
char              *watchlist_filename = NULL;
char              *parse_strategy_filename = NULL;
char              *select_strategy = NULL;
char              *print_strategy = NULL;
HeuristicParms_p  h_parms;
FVIndexParms_p    fvi_parms;
bool              print_sat = false,
   print_full_deriv = false,
   print_statistics = false,
   proof_statistics = false,
   filter_sat = false,
   print_rusage = false,
   print_pid = false,
   print_version = false,
   outinfo = false,
   error_on_empty = false,
   pcl_full_terms = true,
   indexed_subsumption = true,
   syntax_only = false,
   print_formulas = false,
   prune_only = false,
   cnf_only = false,
   inf_sys_complete = true,
   assume_inf_sys_complete = false,
   incomplete = false,
   conjectures_are_questions = false,
   app_encode = false,
   strategy_scheduling = false,
   serialize_schedule = false,
   force_pre_schedule = true;
ProofOutput       print_derivation = PONone;
long              proc_training_data;

IOFormat          parse_format = AutoFormat;
long              step_limit = LONG_MAX,
   answer_limit = 1,
   proc_limit = LONG_MAX,
   unproc_limit = LONG_MAX,
   total_limit = LONG_MAX,
   cores       = 1,
   generated_limit = LONG_MAX,
   relevance_prune_level = 0;
long long tb_insert_limit = LLONG_MAX;
bool lift_lambdas = true;
int num_cpus = 1;
UnifMode unif_mode = SingleUnif;

int force_deriv_output = 0;
char  *outdesc = DEFAULT_OUTPUT_DESCRIPTOR,
      *filterdesc = DEFAULT_FILTER_DESCRIPTOR;
PStack_p          wfcb_definitions, hcb_definitions;
pid_t              pid = 0;
bool               auto_conf = false;
double             clausification_time_part=0.02;

FunctionProperties free_symb_prop = FPIgnoreProps;


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[]);
void print_help(FILE* out);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: set_limits()
//
//   Sets time and memory limits.
//
// Global Variables: -
//
// Side Effects    : Memory, input, may terminate with error.
//
/----------------------------------------------------------------------*/

void set_limits(rlim_t hard_time_limit, rlim_t soft_time_limit, rlim_t mem_limit)
{
   if((hard_time_limit!=RLIM_INFINITY)||(soft_time_limit!=RLIM_INFINITY))
   {
      if(soft_time_limit!=RLIM_INFINITY)
      {
         SetSoftRlimitErr(RLIMIT_CPU, SoftTimeLimit, "RLIMIT_CPU (E-Soft)");
         TimeLimitIsSoft = true;
      }
      else
      {
         SetSoftRlimitErr(RLIMIT_CPU, hard_time_limit, "RLIMIT_CPU (E-Hard)");
         TimeLimitIsSoft = false;
      }

      if(SetSoftRlimit(RLIMIT_CORE, 0)!=RLimSuccess)
      {
         perror("eprover");
         Warning("Cannot prevent core dumps!");
      }
   }
   SetMemoryLimit(mem_limit);
}

/*-----------------------------------------------------------------------
//
// Function: parse_spec()
//
//   Allocate proof state, parse input files into it, and check that
//   requested properties are met. Factored out of main for reasons of
//   readability and length.
//
// Global Variables: -
//
// Side Effects    : Memory, input, may terminate with error.
//
/----------------------------------------------------------------------*/

ProofState_p parse_spec(CLState_p state,
                        IOFormat parse_format_local,
                        bool error_on_empty_local,
                        FunctionProperties free_symb_prop_local,
                        long* ax_no)
{
   ProofState_p proofstate;
   Scanner_p in;
   int i;
   StrTree_p skip_includes = NULL;
   long parsed_ax_no;

   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }
   proofstate = ProofStateAlloc(free_symb_prop_local);
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL, true);
      ScannerSetFormat(in, parse_format_local);
      if(parse_format_local == AutoFormat && in->format == TSTPFormat)
      {
         OutputFormat = TSTPFormat;
         if(DocOutputFormat == no_format)
         {
            DocOutputFormat = tstp_format;
         }
      }
      if(DocOutputFormat==no_format)
      {
         DocOutputFormat = pcl_format;
      }
      FormulaAndClauseSetParse(in,
                               proofstate->f_axioms,
                               proofstate->watchlist,
                               proofstate->terms,
                               NULL,
                               &skip_includes);
      CheckInpTok(in, NoToken);
      DestroyScanner(in);
   }
   VERBOUT2("Specification read\n");

   ProofStateProcessDistinct(proofstate);

   VERBOUT2("$distinct directives processed\n");

   proofstate->has_interpreted_symbols =
      FormulaSetHasInterpretedSymbol(proofstate->f_axioms);
   parsed_ax_no = ProofStateAxNo(proofstate);

   if(error_on_empty_local && (parsed_ax_no == 0))
   {
#ifdef PRINT_SOMEERRORS_STDOUT
      fprintf(GlobalOut, COMCHAR" Error: Input file contains no clauses or formulas\n");
      TSTPOUT(GlobalOut, "InputError");
#endif
      Error("Input file contains no clauses or formulas", OTHER_ERROR);
   }
   *ax_no = parsed_ax_no;

   //printf("Returning set\n");
   return proofstate;
}


/*-----------------------------------------------------------------------
//
// Function: print_info()
//
//   Check if pid and version should be printed, if yes, do so.
//
// Global Variables: print_pid, print_version
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

static void print_info(void)
{
   if(print_pid)
   {
      fprintf(GlobalOut, COMCHAR" Pid: %lld\n", (long long)pid);
      fflush(GlobalOut);
   }
   if(print_version)
   {
      fprintf(GlobalOut, COMCHAR" Version: %s\n", VERSION);
      fflush(GlobalOut);
   }
}


/*-----------------------------------------------------------------------
//
// Function: strategy_io()
//
//   Write and/or read the search strategy parameters. Moved here to
//   declutter main.
//
// Global Variables: -
//
// Side Effects    : I/O, may change h_parms, may terminate the
//                   program (if print_strategy is set).
//
/----------------------------------------------------------------------*/

void strategy_io(HeuristicParms_p h_parms, PStack_p hcb_definitions)
{
   if(parse_strategy_filename)
   {
      Scanner_p in = CreateScanner(StreamTypeFile, parse_strategy_filename,
                                   true, NULL, true);
      HeuristicParmsParseInto(in,h_parms,true);
      if(h_parms->heuristic_def)
      {
         PStackPushP(hcb_definitions, h_parms->heuristic_def);
      }
      DestroyScanner(in);
   }
   if(select_strategy)
   {
      GetHeuristicWithName(select_strategy, h_parms);
   }

   if(print_strategy)
   {
      if(strcmp(print_strategy, ">all-strats<")==0)
      {
         StrategiesPrintPredefined(GlobalOut, false);
      }
      else if(strcmp(print_strategy, ">all-names<")==0)
      {
         StrategiesPrintPredefined(GlobalOut, true);
      }
      else
      {
         if(strcmp(print_strategy, ">current-strategy<")!=0)
         {
            GetHeuristicWithName(print_strategy, h_parms);
         }
         HeuristicParmsPrint(GlobalOut, h_parms);
      }
      exit(NO_ERROR);
   }
}


/*-----------------------------------------------------------------------
//
// Function: handle_auto_mode_preproc()
//
//   Handle (raw) classification and preprocessing scheduling for
//   auto-mode and auto-schedule mode. Moved here to declutter
//   main().
//
// Global Variables: print_rusage, num_cpus, serialize_schedule
//
// Side Effects    : Runs the raw classifier, runs the high-level
//                   scheduler, sets the preprocessing schedule.
//
/----------------------------------------------------------------------*/

int handle_auto_modes_preproc(ProofState_p proofstate,
                              HeuristicParms_p h_parms,
                              Schedule_p *preproc_schedule,
                              SpecLimits_p *limits,
                              RawSpecFeature_p raw_features,
                              rlim_t wc_sched_limit)
{
   int sched_idx = -1;

   *limits = CreateDefaultSpecLimits();

   RawSpecFeaturesCompute(raw_features, proofstate);
   RawSpecFeaturesClassify(raw_features, *limits, RAW_DEFAULT_MASK);
   *preproc_schedule = GetPreprocessingSchedule(raw_features->class);
   fprintf(stdout, COMCHAR" Preprocessing class: %s.\n", raw_features->class);
   if(strategy_scheduling)
   {
      sched_idx = ExecuteScheduleMultiCore(*preproc_schedule, h_parms,
                                           print_rusage,
                                           wc_sched_limit,
                                           true,
                                           num_cpus,
                                           serialize_schedule||(num_cpus==1));
      if (sched_idx != SCHEDULE_DONE)
      {
         char* preproc_conf_name = h_parms->heuristic_name;
         GetHeuristicWithName(preproc_conf_name, h_parms);
      }
      else
      {
         TSTPOUT(GlobalOut, "GaveUp");
         exit(RESOURCE_OUT);
      }
   }
   else
   {
      assert(auto_conf);
      GetHeuristicWithName((*preproc_schedule)->heu_name, h_parms);
      fprintf(stdout, COMCHAR" Configuration: %s\n", (*preproc_schedule)->heu_name);
   }
   return sched_idx;
}



/*-----------------------------------------------------------------------
//
// Function: print_proof_stats()
//
//   Print some statistics about the proof search. This is a pure
//   service function to make main() smaller.
//
// Global Variables: OutputLevel,
//                   print_statistics
//                   GlobalOut,
//                   ClauseClauseSubsumptionCalls,
//                   ClauseClauseSubsumptionCallsRec,
//                   ClauseClauseSubsumptionSuccesses,
//                   UnitClauseClauseSubsumptionCalls,
//                   RewriteUnboundVarFails,
//                   BWRWMatchAttempts,
//                   BWRWMatchSuccesses,
//                   CondensationAttempts,
//                   CondensationSuccesses,
//                   (possibly) UnifAttempts,
//                   (possibly) UnifSuccesses,
//                   (possibly) PDTNodeCounter
//                   (possibly) MguTimer);
//                   (possibly) SatTimer);
//                   (possibly) ParamodTimer);
//                   (possibly) PMIndexTimer);
//                   (possibly) IndexUnifTimer)
//                   (possibly) BWRWTimer);
//                   (possibly) BWRWIndexTimer)
//                   (possibly) IndexMatchTimer
//                   (possibly) FreqVecTimer);
//                   (possibly) FVIndexTimer);
//                   (possibly) SubsumeTimer);
//                   (possibly) SetSubsumeTimer
//
// Side Effects    : Output of collected statistics.
//
/----------------------------------------------------------------------*/

static void print_proof_stats(ProofState_p proofstate,
                              long parsed_ax_no,
                              long relevancy_pruned,
                              long raw_clause_no,
                              long preproc_removed)

{
   if(OutputLevel>1||print_statistics)
   {
      fprintf(GlobalOut, COMCHAR" Parsed axioms                        : %ld\n",
              parsed_ax_no);
      fprintf(GlobalOut, COMCHAR" Removed by relevancy pruning/SinE    : %ld\n",
              relevancy_pruned);
      fprintf(GlobalOut, COMCHAR" Initial clauses                      : %ld\n",
              raw_clause_no);
      fprintf(GlobalOut, COMCHAR" Removed in clause preprocessing      : %ld\n",
              preproc_removed);
      ProofStateStatisticsPrint(GlobalOut, proofstate);
      fprintf(GlobalOut, COMCHAR" Clause-clause subsumption calls (NU) : %ld\n",
              ClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, COMCHAR" Rec. Clause-clause subsumption calls : %ld\n",
              ClauseClauseSubsumptionCallsRec);
      fprintf(GlobalOut, COMCHAR" Non-unit clause-clause subsumptions  : %ld\n",
              ClauseClauseSubsumptionSuccesses);
      fprintf(GlobalOut, COMCHAR" Unit Clause-clause subsumption calls : %ld\n",
              UnitClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, COMCHAR" Rewrite failures with RHS unbound    : %ld\n",
              RewriteUnboundVarFails);
      fprintf(GlobalOut, COMCHAR" BW rewrite match attempts            : %ld\n",
              BWRWMatchAttempts);
      fprintf(GlobalOut, COMCHAR" BW rewrite match successes           : %ld\n",
              BWRWMatchSuccesses);
      fprintf(GlobalOut, COMCHAR" Condensation attempts                : %ld\n",
              CondensationAttempts);
      fprintf(GlobalOut, COMCHAR" Condensation successes               : %ld\n",
              CondensationSuccesses);

#ifdef MEASURE_UNIFICATION
      fprintf(GlobalOut, COMCHAR" Unification attempts                 : %ld\n",
              UnifAttempts);
      fprintf(GlobalOut, COMCHAR" Unification successes                : %ld\n",
              UnifSuccesses);
#endif
#ifdef PDT_COUNT_NODES
      fprintf(GlobalOut, COMCHAR" PDT nodes visited                    : %ld\n",
              PDTNodeCounter);
#endif
      fprintf(GlobalOut, COMCHAR" Termbank termtop insertions          : %lld\n",
              proofstate->terms->insertions);
      fprintf(GlobalOut, COMCHAR" Search garbage collected termcells   : %lld\n",
              proofstate->terms->recovered);
      if(TBPrintDetails)
      {
         TBGCCollect(proofstate->terms);
         fprintf(GlobalOut, COMCHAR" Final garbage collected termcells    : %lld\n",
                 proofstate->terms->recovered);
         fprintf(GlobalOut, COMCHAR" Final shared term nodes              : %ld\n",
                 TBTermNodes(proofstate->terms));
      }
      PERF_CTR_PRINT(GlobalOut, MguTimer);
      PERF_CTR_PRINT(GlobalOut, SatTimer);
      PERF_CTR_PRINT(GlobalOut, ParamodTimer);
      PERF_CTR_PRINT(GlobalOut, PMIndexTimer);
      PERF_CTR_PRINT(GlobalOut, IndexUnifTimer);
      PERF_CTR_PRINT(GlobalOut, BWRWTimer);
      PERF_CTR_PRINT(GlobalOut, BWRWIndexTimer);
      PERF_CTR_PRINT(GlobalOut, IndexMatchTimer);
      PERF_CTR_PRINT(GlobalOut, FreqVecTimer);
      PERF_CTR_PRINT(GlobalOut, FVIndexTimer);
      PERF_CTR_PRINT(GlobalOut, SubsumeTimer);
      PERF_CTR_PRINT(GlobalOut, SetSubsumeTimer);
      PERF_CTR_PRINT(GlobalOut, ClauseEvalTimer);

#ifdef PRINT_INDEX_STATS
      fprintf(GlobalOut, COMCHAR" Backwards rewriting index : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.bw_rw_index);
      fprintf(GlobalOut, "\n");
      /*FPIndexPrintDot(GlobalOut, "rw_bw_index",
        proofstate->gindices.bw_rw_index,
        SubtermTreePrintDot,
        proofstate->signature);*/
      fprintf(GlobalOut, COMCHAR" Paramod-from index        : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_from_index);
      fprintf(GlobalOut, "\n");
      FPIndexPrintDot(GlobalOut, "pm_from_index",
                      proofstate->gindices.pm_from_index,
                      SubtermTreePrintDot,
                      proofstate->signature);
      fprintf(GlobalOut, COMCHAR" Paramod-into index        : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_into_index);
      fprintf(GlobalOut, "\n");
      fprintf(GlobalOut, COMCHAR" Paramod-neg-atom index    : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_negp_index);
      fprintf(GlobalOut, "\n");
#endif
      // PDTreePrint(GlobalOut, proofstate->processed_pos_rules->demod_index);
   }
}


/*-----------------------------------------------------------------------
//
// Function: main()
//
//   Main entry point of the prover. This is where all the cruft
//   accumulates - sorry!
//
// Global Variables: Plenty, mostly flags shared with
//                   process_options. See list above.
//
// Side Effects    : Yes ;-)
//
/----------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
   int              retval = NO_ERROR;
   CLState_p        state;
   ProofState_p     proofstate;
   ProofControl_p   proofcontrol;
   Clause_p         success = NULL,
      filter_success;
   bool             out_of_clauses;
   char             *finals_state = "exists",
      *sat_status = "Derivation";
   long             cnf_size = 0,
      raw_clause_no,
      preproc_removed=0,
      neg_conjectures,
      parsed_ax_no,
      relevancy_pruned = 0;
   double           preproc_time;
   SpecLimits_p spec_limits = NULL;
   RawSpecFeatureCell raw_features;
   SpecFeatureCell features;
   int sched_idx = -1;
   Schedule_p preproc_schedule = NULL;
   rlim_t wc_sched_limit;
   Derivation_p deriv;

   assert(argv[0]);

#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif

   InitIO(NAME);
   pid = getpid();
   //setpgid(0, 0);

   ESignalSetup(SIGXCPU);

   h_parms = HeuristicParmsAlloc();
   fvi_parms = FVIndexParmsAlloc();
   wfcb_definitions = PStackAlloc();
   hcb_definitions = PStackAlloc();

   state = process_options(argc, argv);

   OpenGlobalOut(outname);

   print_info();

   proofstate = parse_spec(state, parse_format,
                           error_on_empty, free_symb_prop,
                           &parsed_ax_no);

   if(syntax_only)
   {
      if(print_formulas)
      {
         FormulaSetPrettyPrintTSTP(GlobalOut, proofstate->f_axioms, true);
      }
      else
      {
         fprintf(GlobalOut, "\n"COMCHAR" Parsing successful!\n");
         TSTPOUT(GlobalOut, "Unknown");
      }
      goto cleanup1;
   }
   wc_sched_limit = ScheduleTimeLimit ? ScheduleTimeLimit : DEFAULT_SCHED_TIME_LIMIT;
   if(auto_conf || strategy_scheduling)
   {
      sched_idx = handle_auto_modes_preproc(proofstate,
                                            h_parms,
                                            &preproc_schedule,
                                            &spec_limits,
                                            &raw_features,
                                            wc_sched_limit);
      CLStateFree(state);
      state = process_options(argc, argv); // refilling the h_parms
                                           // with manual user options
   }

//#ifndef NDEBUG
   fprintf(stdout, COMCHAR" (lift_lambdas = %d, lambda_to_forall = %d,"
           "unroll_only_formulas = %d, sine = %s)\n",
           h_parms->lift_lambdas,
           h_parms->lambda_to_forall,
           h_parms->unroll_only_formulas,
           h_parms->sine);
//#endif

   relevancy_pruned += ProofStateSinE(proofstate, h_parms->sine);
   relevancy_pruned += ProofStateRelevancyProcess(proofstate,
                                                  relevance_prune_level);
   if(app_encode)
   {
      FormulaSetAppEncode(stdout, proofstate->f_axioms);
      goto cleanup1;
   }

   FormulaSetDocInital(GlobalOut, OutputLevel, proofstate->f_axioms);
   ClauseSetDocInital(GlobalOut, OutputLevel, proofstate->axioms);

   if(prune_only)
   {
      fprintf(GlobalOut, "\n"COMCHAR" Pruning successful!\n");
      TSTPOUT(GlobalOut, "Unknown");
      goto cleanup1;
   }

   if(relevancy_pruned || incomplete)
   {
      proofstate->state_is_complete = false;
   }
   FormulaSetArchive(proofstate->f_axioms, proofstate->f_ax_archive);
   //printf("Alive (-2)!\n");
   if((neg_conjectures =
       FormulaSetPreprocConjectures(proofstate->f_axioms,
                                    proofstate->f_ax_archive,
                                    answer_limit>0,
                                    conjectures_are_questions)))
   {
      VERBOUT("Negated conjectures.\n");
   }

   VERBOUT("Clausification started.\n");
   cnf_size = FormulaSetCNF2(proofstate->f_axioms,
                             proofstate->f_ax_archive,
                             proofstate->axioms,
                             proofstate->terms,
                             proofstate->freshvars,
                             h_parms->miniscope_limit,
                             h_parms->formula_def_limit,
                             h_parms->lift_lambdas,
                             h_parms->lambda_to_forall,
                             h_parms->unroll_only_formulas,
                             h_parms->fool_unroll);
   VERBOUT("Clausification done.\n");

   if(cnf_size)
   {
      VERBOUT("CNFization done\n");
   }
   raw_clause_no = proofstate->axioms->members;
   ProofStateLoadWatchlist(proofstate, watchlist_filename, parse_format);

   preproc_removed = ProofStateClausalPreproc(proofstate, h_parms);

   if((strategy_scheduling && sched_idx != -1) || (auto_conf && !cnf_only))
   {
      if(!spec_limits)
      {
         spec_limits = CreateDefaultSpecLimits();
      }
      const int choice_max_depth = h_parms->inst_choice_max_depth;
      SpecFeaturesCompute(&features, proofstate->axioms, proofstate->f_axioms,
                          proofstate->f_ax_archive, proofstate->terms);
      // order info can be affected by clausification
      // (imagine new symbols being introduced)
      features.order = raw_features.order;
      features.goal_order = raw_features.conj_order;
      features.num_of_definitions = raw_features.num_of_definitions;
      features.perc_of_form_defs = raw_features.perc_of_form_defs;
      SpecFeaturesAddEval(&features, spec_limits);
      char* class = SpecTypeString(&features, DEFAULT_MASK);
      fprintf(stdout, COMCHAR" Search class: %s\n", class);
      if (strategy_scheduling)
      {
         set_limits(HardTimeLimit, SoftTimeLimit, h_parms->mem_limit);
         ScheduleCell* search_sched = GetSearchSchedule(class);
         InitializePlaceholderSearchSchedule(search_sched, preproc_schedule+sched_idx,
                                             force_pre_schedule);
         int status =
            ExecuteScheduleMultiCore(search_sched,
                                     h_parms, print_rusage,
                                     preproc_schedule[sched_idx].time_absolute,
                                     false, preproc_schedule[sched_idx].cores, false);
         if (status == SCHEDULE_DONE)
         {
            double total_cpu = GetTotalCPUTimeIncludingChildren();
            double total_limit = preproc_schedule[sched_idx].time_absolute;
            double remaining_time = total_limit - total_cpu;
            if(remaining_time > RETRY_DEFAULT_SCHEDULE_THRESHOLD)
            {
               ScheduleCell* filtered_default = GetFilteredDefaultSchedule(search_sched);
#ifdef NDEBUG
               FILE* out = stdout;
#else
               FILE* out = stderr;
#endif
               fprintf(out, COMCHAR" executing default schedule for %g seconds.\n", remaining_time);
               status =
                  ExecuteScheduleMultiCore(filtered_default, h_parms, print_rusage,
                                          remaining_time, false,
                                          preproc_schedule[sched_idx].cores, false);
               if (status == SCHEDULE_DONE)
               {
                  exit(RESOURCE_OUT);
               }
            }
            else
            {
               exit(RESOURCE_OUT);
            }
         }
         GetHeuristicWithName(h_parms->heuristic_name, h_parms);
         h_parms->inst_choice_max_depth = choice_max_depth;
      }
      else
      {
         assert(auto_conf);
         // executing the first one from the schedule.
         char* conf_name = GetSearchSchedule(class)->heu_name;
         GetHeuristicWithName(conf_name, h_parms);
         fprintf(stdout, COMCHAR" Configuration: %s\n", conf_name);
         // STS: ASK PV About this!
         h_parms->inst_choice_max_depth = choice_max_depth;
      }
      FREE(class);
      CLStateFree(state);
      state = process_options(argc, argv); // refilling the h_parms with user options
   }
   strategy_io(h_parms, hcb_definitions);

   if(spec_limits)
   {
      SpecLimitsCellFree(spec_limits);
   }
   proofcontrol = ProofControlAlloc();
   ProofControlInit(proofstate, proofcontrol, h_parms,
                    fvi_parms, wfcb_definitions, hcb_definitions);

   // Unfold definitions and re-normalize
   PCLFullTerms = pcl_full_terms; /* Preprocessing always uses full
                                     terms, so we set the flag for
                                     the main proof search only now! */
   GlobalIndicesInit(&(proofstate->wlindices),
                     proofstate->signature,
                     proofcontrol->heuristic_parms.rw_bw_index_type,
                     "NoIndex",
                     "NoIndex",
                     proofcontrol->heuristic_parms.ext_rules_max_depth);
   //printf("Alive (1)!\n");

   ProofStateInit(proofstate, proofcontrol);
   //printf("Alive (2)!\n");

   VERBOUT2("Prover state initialized\n");
   preproc_time = GetTotalCPUTime();
   if(print_rusage)
   {
      fprintf(GlobalOut, COMCHAR" Preprocessing time       : %.3f s\n", preproc_time);
   }

   if(proofcontrol->heuristic_parms.presat_interreduction)
   {
      LiteralSelectionFun sel_strat =
         proofcontrol->heuristic_parms.selection_strategy;

      proofcontrol->heuristic_parms.selection_strategy = SelectNoGeneration;
      success = Saturate(proofstate, proofcontrol, LONG_MAX,
                         LONG_MAX, LONG_MAX, LONG_MAX, LONG_MAX,
                         LLONG_MAX, LONG_MAX);
      fprintf(GlobalOut, COMCHAR" Presaturation interreduction done\n");
      proofcontrol->heuristic_parms.selection_strategy = sel_strat;
      if(!success)
      {
         ProofStateResetProcessed(proofstate, proofcontrol);
      }
   }
   PERF_CTR_ENTRY(SatTimer);


   if(SigHasUnimplementedInterpretedSymbols(proofstate->signature)||
      (proofcontrol->heuristic_parms.selection_strategy ==  SelectNoGeneration) ||
      (proofcontrol->heuristic_parms.order_params.lit_cmp == LCTFOEqMax)||
      (!h_parms->enable_eq_factoring)||
      (!h_parms->enable_neg_unit_paramod))
   {
      inf_sys_complete = false;
   }

   if(!success)
   {
      success = Saturate(proofstate, proofcontrol, step_limit,
                         proc_limit, unproc_limit, total_limit,
                         generated_limit, tb_insert_limit, answer_limit);
   }
   PERF_CTR_EXIT(SatTimer);

   out_of_clauses = ClauseSetEmpty(proofstate->unprocessed);
   if(filter_sat)
   {
      filter_success = ProofStateFilterUnprocessed(proofstate,
                                                   proofcontrol,
                                                   filterdesc);
      if(filter_success)
      {
         success = filter_success;
         PStackPushP(proofstate->extract_roots, success);
      }
   }

   if(success||proofstate->answer_count)
   {
      assert(!PStackEmpty(proofstate->extract_roots));
      if(success)
      {
         DocClauseQuoteDefault(2, success, "proof");
      }


      fprintf(GlobalOut, "\n"COMCHAR" Proof found!\n");

      if(print_full_deriv)
      {
         ClauseSetPushClauses(proofstate->extract_roots,
                                 proofstate->processed_pos_rules);
         ClauseSetPushClauses(proofstate->extract_roots,
                                 proofstate->processed_pos_eqns);
         ClauseSetPushClauses(proofstate->extract_roots,
                                 proofstate->processed_neg_units);
         ClauseSetPushClauses(proofstate->extract_roots,
                                 proofstate->processed_non_units);
         ClauseSetPushClauses(proofstate->extract_roots,
                                 proofstate->unprocessed);
      }
      deriv = DerivationCompute(proofstate->extract_roots,
                              proofstate->signature);

      if(!proofstate->status_reported)
      {
         if(neg_conjectures)
         {
            TSTPOUT(GlobalOut, deriv->has_conjecture?"Theorem":"ContradictoryAxioms");
         }
         else
         {
            TSTPOUT(GlobalOut, "Unsatisfiable");
         }
         proofstate->status_reported = true;
         retval = PROOF_FOUND;
      }


      if(PrintProofObject)
      {
         DerivationPrintConditional(GlobalOut,
                                    "CNFRefutation",
                                    deriv,
                                    proofstate->signature,
                                    print_derivation,
                                    proof_statistics);
         ProofStateAnalyseGC(proofstate);
         if(proc_training_data)
         {
            ProofStateTrain(proofstate, proc_training_data&TSPrintPos,
                        proc_training_data&TSPrintNeg);
         }
      }
      DerivationFree(deriv);
   }
   else if(proofstate->watchlist && ClauseSetEmpty(proofstate->watchlist))
   {
      ProofStatePropDocQuote(GlobalOut, OutputLevel,
                             CPSubsumesWatch, proofstate,
                             "final_subsumes_wl");
      fprintf(GlobalOut, "\n"COMCHAR" Watchlist is empty!\n");
      TSTPOUT(GlobalOut, "ResourceOut");
      retval = RESOURCE_OUT;
   }
   else
   {
      if(out_of_clauses&&
         proofstate->state_is_complete&&
         (inf_sys_complete || assume_inf_sys_complete))
      {
         finals_state = "final";
      }
      ProofStatePropDocQuote(GlobalOut, OutputLevel, CPIgnoreProps,
                             proofstate, finals_state);

      if(cnf_only)
      {
         fprintf(GlobalOut, "\n"COMCHAR" CNFization successful!\n");
         TSTPOUT(GlobalOut, "Unknown");
      }
      else if(out_of_clauses)
      {
         if(!(inf_sys_complete || assume_inf_sys_complete))
         {
            fprintf(GlobalOut,
                    "\n"COMCHAR" Clause set closed under "
                    "restricted calculus!\n");
            if(!SilentTimeOut)
            {
               TSTPOUT(GlobalOut, "GaveUp");
            }
            retval = INCOMPLETE_PROOFSTATE;
         }
         else if(proofstate->state_is_complete &&
                 inf_sys_complete &&
                 proofstate->has_interpreted_symbols)
         {
            fprintf(GlobalOut,
                    "\n"COMCHAR" Clause set saturated up to interpreted theories!\n");
            if(!SilentTimeOut)
            {
               TSTPOUT(GlobalOut, "GaveUp");
            }
            retval = INCOMPLETE_PROOFSTATE;
         }
         else if(problemType != PROBLEM_HO
                 && proofstate->state_is_complete
                 && inf_sys_complete)
         {
            fprintf(GlobalOut, "\n"COMCHAR" No proof found!\n");
            TSTPOUT(GlobalOut, neg_conjectures?"CounterSatisfiable":"Satisfiable");
            sat_status = "Saturation";
            retval = SATISFIABLE;
         }
         else
         {
            fprintf(GlobalOut, "\n"COMCHAR" Failure: Out of unprocessed clauses!\n");
            if(!SilentTimeOut)
            {
               //ClauseSetPrint(stderr, proofstate->processed_pos_rules, true);
               //ClauseSetPrint(stderr, proofstate->processed_pos_eqns, true);
               //ClauseSetPrint(stderr, proofstate->processed_neg_units, true);
               //ClauseSetPrint(stderr, proofstate->processed_non_units, true);
               TSTPOUT(GlobalOut, "GaveUp");
            }
            retval = INCOMPLETE_PROOFSTATE;
         }
      }
      else
      {
         fprintf(GlobalOut, "\n"COMCHAR" Failure: User resource limit exceeded!\n");
         if(!SilentTimeOut)
         {
            TSTPOUT(GlobalOut, "ResourceOut");
         }
         retval = RESOURCE_OUT;
      }
      if(PrintProofObject &&
         (((retval!=INCOMPLETE_PROOFSTATE)&&
           (retval!=RESOURCE_OUT))
          |force_deriv_output))
      {
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_pos_rules);
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_pos_eqns);
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_neg_units);
         ClauseSetPushClauses(proofstate->extract_roots,
                              proofstate->processed_non_units);
         if(cnf_only|(force_deriv_output>=2))
         {
            ClauseSetPushClauses(proofstate->extract_roots,
                                 proofstate->unprocessed);
            print_sat = false;
         }
         DerivationComputeAndPrint(GlobalOut,
                                   sat_status,
                                   proofstate->extract_roots,
                                   proofstate->signature,
                                   print_derivation,
                                   proof_statistics);
      }

   }
   /* ClauseSetDerivationStackStatistics(proofstate->unprocessed); */
   if(print_sat)
   {
      if(proofstate->non_redundant_deleted)
      {
         fprintf(GlobalOut, "\n"COMCHAR" Saturated system is incomplete!\n");
      }
      if(success)
      {
         fprintf(GlobalOut, COMCHAR" Saturated system contains the empty clause:\n");
         ClausePrint(GlobalOut, success, true);
         fputc('\n',GlobalOut);
         fputc('\n',GlobalOut);
      }
      ProofStatePrintSelective(GlobalOut, proofstate, outdesc,
                               outinfo);
      fprintf(GlobalOut, "\n");
   }

   if(success)
   {
      ClauseFree(success);
   }
   fflush(GlobalOut);

   print_proof_stats(proofstate,
                     parsed_ax_no,
                     relevancy_pruned,
                     raw_clause_no,
                     preproc_removed);
#ifndef FAST_EXIT
#ifdef FULL_MEM_STATS
   fprintf(GlobalOut,
           COMCHAR" sizeof TermCell     : %ld\n"
           COMCHAR" sizeof EqnCell      : %ld\n"
           COMCHAR" sizeof ClauseCell   : %ld\n"
           COMCHAR" sizeof PTreeCell    : %ld\n"
           COMCHAR" sizeof PDTNodeCell  : %ld\n"
           COMCHAR" sizeof EvalCell     : %ld\n"
           COMCHAR" sizeof ClausePosCell: %ld\n"
           COMCHAR" sizeof PDArrayCell  : %ld\n",
           sizeof(TermCell),
           sizeof(EqnCell),
           sizeof(ClauseCell),
           sizeof(PTreeCell),
           sizeof(PDTNodeCell),
           sizeof(EvalCell),
           sizeof(ClausePosCell),
           sizeof(PDArrayCell));
   fprintf(GlobalOut, COMCHAR"Estimated memory usage: %ld\n",
           ProofStateStorage(proofstate));
   MemFreeListPrint(GlobalOut);
#endif
   ProofControlFree(proofcontrol);
#endif
cleanup1:
#ifndef FAST_EXIT
   ProofStateFree(proofstate);
   CLStateFree(state);
   PStackFree(hcb_definitions);
   PStackFree(wfcb_definitions);
   FVIndexParmsFree(fvi_parms);
   HeuristicParmsFree(h_parms);
   PermaStringsFree();
#ifdef FULL_MEM_STATS
   MemFreeListPrint(GlobalOut);
#endif
#endif
   if(print_rusage && !SilentTimeOut)
   {
      PrintRusage(GlobalOut);
   }
#ifdef CLB_MEMORY_DEBUG
   RegMemCleanUp();
   MemFlushFreeList();
   MemDebugPrintStats(stdout);
#endif
   OutClose(GlobalOut);
   return retval;
}


/*-----------------------------------------------------------------------
//
// Function: check_fp_index_arg()
//
//   Check in arg is a valid term describing a FP-index function. If
//   yes, return true. If no, print error (nominally return false).
//
// Global Variables: -
//
// Side Effects    : May terminate program with error.
//
/----------------------------------------------------------------------*/

bool check_fp_index_arg(char* arg, char* opt)
{
   DStr_p err;

   if(GetFPIndexFunction(arg)||(strcmp(arg, "NoIndex")==0))
   {
      return true;
   }
   err = DStrAlloc();
   DStrAppendStr(err,
                 "Wrong argument to option ");
   DStrAppendStr(err,
                 opt);
   DStrAppendStr(err,
                 ". Possible values: ");
   DStrAppendStrArray(err, FPIndexNames, ", ");
   Error(DStrView(err), USAGE_ERROR);
   DStrFree(err);

   return false;
}


/*-----------------------------------------------------------------------
//
// Function: process_options()
//
//   Read and process the command line option, return (the pointer to)
//   a CLState object containing the remaining arguments.
//
// Global Variables: opts, Verbose, TBPrintInternalInfo
//
// Side Effects    : Sets variables, may terminate with program
//                   description if option -h or --help was present
//
/----------------------------------------------------------------------*/

CLState_p process_options(int argc, char* argv[])
{
   Opt_p handle;
   CLState_p state;
   char*  arg;
   long   tmp;
   rlim_t mem_limit;

   state = CLStateAlloc(argc,argv);

   while((handle = CLStateGetOpt(state, &arg, opts)))
   {
      switch(handle->option_code)
      {
      case OPT_VERBOSE:
            Verbose = CLStateGetIntArg(handle, arg);
            break;
      case OPT_HELP:
            print_help(stdout);
            exit(NO_ERROR);
      case OPT_VERSION:
            fprintf(stdout, "E %s %s (%s)\n", VERSION, E_NICKNAME, ECOMMITID);
            exit(NO_ERROR);
      case OPT_OUTPUT:
            outname = arg;
            break;
      case OPT_SILENT:
            OutputLevel = 0;
            break;
      case OPT_OUTPUTLEVEL:
            OutputLevel = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PROOF_OBJECT:
            PrintProofObject = MAX(CLStateGetIntArgCheckRange(handle, arg, 0, 3),
                                   PrintProofObject);
            print_derivation = MAX(print_derivation, POList);
            break;
      case OPT_PROOF_STATS:
            proof_statistics = true;
            break;
      case OPT_PROOF_GRAPH:
            PrintProofObject = MAX(1, PrintProofObject);
            print_derivation = CLStateGetIntArg(handle, arg)+1;
            break;
      case OPT_FULL_DERIV:
            print_full_deriv = true;
            break;
      case OPT_FORCE_DERIV:
            force_deriv_output = CLStateGetIntArgCheckRange(handle, arg, 0, 2);
            PrintProofObject = MAX(1, PrintProofObject);
            break;
      case OPT_RECORD_GIVEN_CLAUSES:
            PrintProofObject = MAX(1, PrintProofObject);
            ProofObjectRecordsGCSelection = true;
            break;
      case OPT_TRAINING:
            PrintProofObject = MAX(1, PrintProofObject);
            ProofObjectRecordsGCSelection = true;
            proc_training_data = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PCL_COMPRESSED:
            pcl_full_terms = false;
            break;
      case OPT_PCL_COMPACT:
            PCLStepCompact = true;
            break;
      case OPT_PCL_SHELL_LEVEL:
            PCLShellLevel =  CLStateGetIntArgCheckRange(handle, arg, 0, 2);
            break;
      case OPT_PRINT_STATISTICS:
            print_statistics = true;
            break;
      case OPT_EXPENSIVE_DETAILS:
            TBPrintDetails = true;
            print_statistics = true;
            break;
      case OPT_PRINT_SATURATED:
            outdesc = arg;
            CheckOptionLetterString(outdesc, "teigEIGaA", "-S (--print-saturated)");
            print_sat = true;
            break;
      case OPT_PRINT_SAT_INFO:
            outinfo = true;
            break;
      case OPT_FILTER_SATURATED:
            filterdesc = arg;
            CheckOptionLetterString(filterdesc, "eigEIGaA", "--filter-saturated");
            filter_sat = true;
            break;
      case OPT_SYNTAX_ONLY:
            syntax_only = true;
            break;
      case OPT_PRINT_FORMULAS:
            syntax_only = true;
            print_formulas = true;
            break;
      case OPT_PRUNE_ONLY:
            OutputLevel = 4;
            prune_only  = true;
            break;
      case OPT_CNF_ONLY:
            outdesc    = "teigEIG";
            print_sat  = true;
            proc_limit = 0;
            cnf_only   = true;
            break;
      case OPT_PRINT_PID:
            print_pid = true;
            break;
      case OPT_PRINT_VERSION:
            print_version = true;
            break;
      case OPT_REQUIRE_NONEMPTY:
            error_on_empty = true;
            break;
      case OPT_MEM_LIMIT:
            if(strcmp(arg, "Auto")==0)
            {
               long long tmpmem = GetSystemPhysMemory();

               if(tmpmem==-1)
               {
                  Error("Cannot find physical memory automatically. "
                        "Give explicit value to --memory-limit", OTHER_ERROR);
               }
               VERBOSE(fprintf(stderr,
                               "Physical memory determined as %lld MB\n",
                               tmpmem););

               mem_limit = 0.8*tmpmem;

               h_parms->delete_bad_limit =
                  (float)(mem_limit-2)*0.7*MEGA;
            }
            else
            {
               /* We expect the user to know what he is doing */
               mem_limit = CLStateGetIntArg(handle, arg);
            }
            VERBOSE(fprintf(stderr,
                            "Memory limit set to %lld MB\n",
                            (long long)mem_limit););
            h_parms->mem_limit = MEGA*mem_limit;
            break;
      case OPT_CPU_LIMIT:
            HardTimeLimit = CLStateGetIntArg(handle, arg);
            ScheduleTimeLimit = HardTimeLimit;
            if((SoftTimeLimit != RLIM_INFINITY) &&
               (HardTimeLimit<=SoftTimeLimit))
            {
               Error("Hard time limit has to be larger than soft"
                     "time limit", USAGE_ERROR);
            }
            break;
      case OPT_SOFTCPU_LIMIT:
            SoftTimeLimit = CLStateGetIntArg(handle, arg);
            ScheduleTimeLimit = SoftTimeLimit;

            if((HardTimeLimit != RLIM_INFINITY) &&
               (HardTimeLimit<=SoftTimeLimit))
            {
               Error("Soft time limit has to be smaller than hard"
                     "time limit", USAGE_ERROR);
            }
            break;
      case OPT_RUSAGE_INFO:
            print_rusage = true;
            break;
      case OPT_SELECT_STRATEGY:
            select_strategy = arg;
            break;
      case OPT_PRINT_STRATEGY:
            print_strategy = arg;
            break;
      case OPT_PARSE_STRATEGY:
            parse_strategy_filename = arg;
            break;
      case OPT_STEP_LIMIT:
            step_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ANSWER_LIMIT:
            answer_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_CONJ_ARE_QUEST:
            conjectures_are_questions = true;
            break;
      case OPT_PROC_LIMIT:
            proc_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_UNPROC_LIMIT:
            unproc_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TOTAL_LIMIT:
            total_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_GENERATED_LIMIT:
            generated_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TB_INSERT_LIMIT:
            tb_insert_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NO_INFIX:
            EqnUseInfix = false;
            break;
      case OPT_FULL_EQ_REP:
            EqnFullEquationalRep = true;
            break;
      case OPT_LOP_PARSE:
            parse_format = LOPFormat;
            break;
      case OPT_PCL_PRINT:
            DocOutputFormat = pcl_format;
            break;
      case OPT_TPTP_PRINT:
            OutputFormat = TPTPFormat;
            EqnFullEquationalRep = false;
            EqnUseInfix = false;
            break;
      case OPT_TPTP_FORMAT:
            parse_format = TPTPFormat;
            OutputFormat = TPTPFormat;
            EqnFullEquationalRep = false;
            EqnUseInfix = false;
            break;
      case OPT_TSTP_PARSE:
            parse_format = TSTPFormat;
            break;
      case OPT_TSTP_PRINT:
            DocOutputFormat = tstp_format;
            OutputFormat = TSTPFormat;
            EqnUseInfix = true;
            break;
      case OPT_TSTP_FORMAT:
            parse_format = TSTPFormat;
            DocOutputFormat = tstp_format;
            OutputFormat = TSTPFormat;
            EqnUseInfix = true;
            break;
      case OPT_AUTO:
            if(!auto_conf)
            {
               h_parms->sine = "Auto";
               auto_conf = true;
            }
            break;
      case OPT_AUTO_SCHED:
            if(!strategy_scheduling)
            {
               if(strcmp(arg, "Auto")==0)
               {
                  num_cpus = -1;
               }
               else
               {
                  num_cpus = CLStateGetIntArg(handle, arg);
               }
               h_parms->sine = "Auto";
               strategy_scheduling = true;
            }
            break;
      case OPT_SERIALIZE_SCHEDULE:
            serialize_schedule = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_FORCE_PREPROC_SCHED:
            force_pre_schedule = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_SATAUTO_SCHED:
            if(!strategy_scheduling)
            {
               if(strcmp(arg, "Auto")==0)
               {
                  num_cpus = -1;
               }
               else
               {
                  num_cpus = CLStateGetIntArg(handle, arg);
               }
               strategy_scheduling = true;
            }
            break;
      case OPT_NO_PREPROCESSING:
            h_parms->no_preproc = true;
            break;
      case OPT_EQ_UNFOLD_LIMIT:
            h_parms->eqdef_incrlimit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_EQ_UNFOLD_MAXCLAUSES:
            h_parms->eqdef_maxclauses = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NO_EQ_UNFOLD:
            h_parms->eqdef_incrlimit = LONG_MIN;
            break;
      case OPT_INTRO_GOAL_DEFS:
            if(strcmp(arg, "None")==0)
            {
               h_parms->add_goal_defs_pos = false;
               h_parms->add_goal_defs_neg = false;
            }
            else if(strcmp(arg, "All")==0)
            {
               h_parms->add_goal_defs_pos = true;
               h_parms->add_goal_defs_neg = true;
            }
            else if(strcmp(arg, "Neg")==0)
            {
               h_parms->add_goal_defs_pos = false;
               h_parms->add_goal_defs_neg = true;
            }
            else
            {
                Error("Option --goal-defs accepts only None, All, or Neg",
                     USAGE_ERROR);
            }
            break;
      case OPT_FINE_GOAL_DEFS:
            h_parms->add_goal_defs_subterms = true;
            break;
      case OPT_SINE:
            //h_parms->sine = "Auto";
            h_parms->sine = arg;
            break;
      case OPT_REL_PRUNE_LEVEL:
            relevance_prune_level = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PRESAT_SIMPLIY:
            h_parms->presat_interreduction = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_AC_HANDLING:
            if(strcmp(arg, "None")==0)
            {
               h_parms->ac_handling = NoACHandling;
            }
            else if(strcmp(arg, "DiscardAll")==0)
            {
               h_parms->ac_handling = ACDiscardAll;
            }
            else if(strcmp(arg, "KeepUnits")==0)
            {
               h_parms->ac_handling = ACKeepUnits;
            }
            else if(strcmp(arg, "KeepOrientable")==0)
            {
               h_parms->ac_handling = ACKeepOrientable;
            }
            else
            {
               Error("Option --ac_handling requires None, DiscardAll, "
                     "KeepUnits, or KeepOrientable as an argument",
                     USAGE_ERROR);
            }
            break;
      case OPT_AC_ON_PROC:
            h_parms->ac_res_aggressive = false;
            break;
      case OPT_NO_GENERATION:
            h_parms->selection_strategy=SelectNoGeneration;
            break;
      case OPT_SELECT_ON_PROC_ONLY:
            h_parms->select_on_proc_only = true;
            break;
      case OPT_INHERIT_PM_LIT:
            h_parms->inherit_paramod_lit = true;
            break;
      case OPT_INHERIT_GOAL_PM_LIT:
            h_parms->inherit_goal_pm_lit = true;
            break;
      case OPT_INHERIT_CONJ_PM_LIT:
            h_parms->inherit_conj_pm_lit = true;
            break;
      case OPT_LITERAL_SELECT:
            h_parms->selection_strategy = GetLitSelFun(arg);
            if(!h_parms->selection_strategy)
            {
               DStr_p err = DStrAlloc();
               DStrAppendStr(err,
                             "Wrong argument to option -W "
                             "(--literal-selection-strategy). Possible "
                             "values: ");
               LitSelAppendNames(err);
               Error(DStrView(err), USAGE_ERROR);
               DStrFree(err);
            }
            // Incomplete selection is noted later
            break;
      case OPT_POS_LITSEL_MIN:
            h_parms->pos_lit_sel_min = CLStateGetIntArg(handle, arg);
            break;
      case OPT_POS_LITSEL_MAX:
            h_parms->pos_lit_sel_max = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NEG_LITSEL_MIN:
            h_parms->neg_lit_sel_min = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NEG_LITSEL_MAX:
            h_parms->neg_lit_sel_max = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ALL_LITSEL_MIN:
            h_parms->all_lit_sel_min = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ALL_LITSEL_MAX:
            h_parms->all_lit_sel_max = CLStateGetIntArg(handle, arg);
            break;
      case OPT_WEIGHT_LITSEL_MIN:
            h_parms->weight_sel_min = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PREFER_INITIAL_CLAUSES:
            h_parms->prefer_initial_clauses = true;
            break;
      case OPT_HEURISTIC:
            h_parms->heuristic_name = arg;
            break;
      case OPT_FILTER_ORPHANS_LIMIT:
            h_parms->filter_orphans_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FORWARD_CONTRACT_LIMIT:
            h_parms->forward_contract_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_DELETE_BAD_LIMIT:
            h_parms->delete_bad_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ASSUME_COMPLETENESS:
            assume_inf_sys_complete = true;
            break;
      case OPT_ASSUME_INCOMPLETENESS:
            incomplete = true;
            break;
      case OPT_NO_GC_FORWARD_SIMPL:
            h_parms->enable_given_forward_simpl = false;
            break;
      case OPT_DISABLE_EQ_FACTORING:
            h_parms->enable_eq_factoring = false;
            inf_sys_complete = false;
            break;
      case OPT_DISABLE_NEGUNIT_PM:
            h_parms->enable_neg_unit_paramod = false;
            inf_sys_complete = false;
            break;
      case OPT_CONDENSING:
            h_parms->condensing = true;
            break;
      case OPT_CONDENSING_AGGRESSIVE:
            h_parms->condensing = true;
            h_parms->condensing_aggressive = true;
            break;
      case OPT_USE_SIM_PARAMOD:
            h_parms->pm_type = ParamodSim;
            break;
      case OPT_USE_ORIENTED_SIM_PARAMOD:
            h_parms->pm_type = ParamodOrientedSim;
            break;
      case OPT_USE_SUPERSIM_PARAMOD:
            h_parms->pm_type = ParamodSuperSim;
            break;
      case OPT_USE_ORIENTED_SUPERSIM_PARAMOD:
            h_parms->pm_type = ParamodOrientedSuperSim;
            break;
      case OPT_SPLIT_TYPES:
            h_parms->split_clauses = CLStateGetIntArg(handle, arg);
            break;
      case OPT_SPLIT_HOW:
            h_parms->split_method = CLStateGetIntArgCheckRange(handle, arg, 0, 2);
            break;
      case OPT_SPLIT_AGGRESSIVE:
            h_parms->split_aggressive = true;
            break;
      case OPT_SPLIT_REUSE_DEFS:
            h_parms->split_fresh_defs = false;
            break;
      case OPT_DISEQ_DECOMPOSITION:
            h_parms->diseq_decomposition = CLStateGetIntArg(handle, arg);
            break;
      case OPT_DISEQ_DECOMP_MAXARITY:
            h_parms->diseq_decomp_maxarity = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ORDERING:
            if(strcmp(arg, "LPO")==0)
            {
               h_parms->order_params.ordertype = LPO;
            }
            else if(strcmp(arg, "LPOCopy")==0)
            {
               h_parms->order_params.ordertype = LPOCopy;
            }
            else if(strcmp(arg, "LPO4")==0)
            {
               h_parms->order_params.ordertype = LPO4;
            }
            else if(strcmp(arg, "LPO4Copy")==0)
            {
               h_parms->order_params.ordertype = LPO4Copy;
            }
            else if(strcmp(arg, "KBO")==0)
            {
               h_parms->order_params.ordertype = KBO;
            }
            else if(strcmp(arg, "KBO6")==0)
            {
               h_parms->order_params.ordertype = KBO6;
            }
            /* else if(strcmp(arg, "Optimize")==0) */
            /* { */
            /*    h_parms->order_params.ordertype = OPTIMIZE_AX; */
            /* } */
            else
            {
               Error("Option -t (--term-ordering) requires "
                     "LPO, LPO4, KBO or KBO6 as an argument",
                     USAGE_ERROR);
            }
            break;
      case OPT_LAM_WEIGHT:
            h_parms->order_params.lam_w = CLStateGetIntArg(handle, arg);
            break;
      case OPT_DB_WEIGHT:
            h_parms->order_params.db_w = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TO_WEIGHTGEN:
            h_parms->order_params.to_weight_gen = TOTranslateWeightGenMethod(arg);
            if(!h_parms->order_params.to_weight_gen)
            {
               DStr_p err = DStrAlloc();
               DStrAppendStr(err,
                             "Wrong argument to option -w "
                             "(--order-weight-generation). Possible "
                             "values: ");
               DStrAppendStrArray(err, TOWeightGenNames, ", ");
               Error(DStrView(err), USAGE_ERROR);
               DStrFree(err);
            }
            break;
      case OPT_TO_WEIGHTS:
            h_parms->order_params.to_pre_weights = arg;
            break;
      case OPT_TO_PRECGEN:
            h_parms->order_params.to_prec_gen = TOTranslatePrecGenMethod(arg);
            if(!h_parms->order_params.to_prec_gen)
            {
               DStr_p err = DStrAlloc();
               DStrAppendStr(err,
                             "Wrong argument to option -G "
                             "(--order-precedence-generation). Possible "
                             "values: ");
               DStrAppendStrArray(err, TOPrecGenNames, ", ");
               Error(DStrView(err), USAGE_ERROR);
               DStrFree(err);
            }
            break;
      case OPT_TO_CONJONLY_PREC:
            h_parms->order_params.conj_only_mod = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TO_CONJAXIOM_PREC:
            h_parms->order_params.conj_axiom_mod = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TO_AXIOMONLY_PREC:
            h_parms->order_params.axiom_only_mod = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TO_SKOLEM_PREC:
            h_parms->order_params.skolem_mod = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TO_DEFPRED_PREC:
            h_parms->order_params.defpred_mod = CLStateGetIntArg(handle, arg);
            break;
      case OPT_TO_CONSTWEIGHT:
            h_parms->order_params.to_const_weight = CLStateGetIntArg(handle, arg);
            if(h_parms->order_params.to_const_weight<=0)
            {
               Error("Argument to option -c (--order-constant-weight) "
                     "has to be > 0", USAGE_ERROR);
            }
            break;
      case OPT_TO_PRECEDENCE:
            h_parms->order_params.to_pre_prec = arg;
            break;
      case OPT_TO_LPO_RECLIMIT:
            LPORecursionDepthLimit = CLStateGetIntArg(handle, arg);
            if(LPORecursionDepthLimit<=0)
            {
               Error("Argument to option --lpo-recursion-limit "
                     "has to be > 0", USAGE_ERROR);
            }
            if(LPORecursionDepthLimit>20000)
            {
               Warning("Using very large values for "
                       "--lpo-recursion-limit may lead to stack "
                       "overflows and segmentation faults.");
            }
      case OPT_TO_RESTRICT_LIT_CMPS:
            h_parms->order_params.lit_cmp = LCNoCmp;
            break;
      case OPT_TO_LIT_CMP:
            if(strcmp(arg, "None")==0)
            {
               h_parms->order_params.lit_cmp = LCNoCmp;
            }
            else if(strcmp(arg, "Normal")==0)
            {
               h_parms->order_params.lit_cmp = LCNormal;
            }
            else if(strcmp(arg, "TFOEqMax")==0)
            {
               h_parms->order_params.lit_cmp = LCTFOEqMax;
            }
            else if(strcmp(arg, "TFOEqMin")==0)
            {
               h_parms->order_params.lit_cmp = LCTFOEqMin;
            }
            else
            {
               Error("Wrong argument to --literal-comparison (valid: "
                     "None, Normal, TFOEqMax, TFOEqMin).", USAGE_ERROR);
            }
            break;
      case OPT_TPTP_SOS:
            h_parms->use_tptp_sos = true;
            break;
      case OPT_ER_DESTRUCTIVE:
            h_parms->er_varlit_destructive = true;
            break;
      case OPT_ER_STRONG_DESTRUCTIVE:
            h_parms->er_varlit_destructive = true; /* Implied */
            h_parms->er_strong_destructive = true;
            break;
      case OPT_ER_AGGRESSIVE:
            h_parms->er_aggressive = true;
            break;
      case OPT_FORWARD_CSR:
            h_parms->forward_context_sr = true;
            break;
      case OPT_FORWARD_CSR_AGGRESSIVE:
            h_parms->forward_context_sr = true;
            h_parms->forward_context_sr_aggressive = true;
            break;
      case OPT_BACKWARD_CSR:
            h_parms->backward_context_sr = true;
            break;
      case OPT_RULES_GENERAL:
            h_parms->prefer_general = true;
            break;
      case OPT_FORWARD_DEMOD:
            h_parms->forward_demod = CLStateGetIntArgCheckRange(handle, arg, 0, 2);
            break;
      case OPT_DEMOD_LAMBDA:
            h_parms->lambda_demod = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_LIFT_LAMBDAS:
            h_parms->lift_lambdas = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_STRONG_RHS_INSTANCE:
            h_parms->order_params.rewrite_strong_rhs_inst = true;
            break;
      case OPT_STRONGSUBSUMPTION:
            StrongUnitForwardSubsumption = true;
            break;
      case OPT_SAT_STEP_INTERVAL:
            h_parms->sat_check_step_limit =
               CLStateGetIntArgCheckRange(handle, arg, 1, LONG_MAX);
            break;
      case OPT_SAT_SIZE_INTERVAL:
            h_parms->sat_check_size_limit =
               CLStateGetIntArgCheckRange(handle, arg, 1, LONG_MAX);
            break;
      case OPT_SAT_TTINSERT_INTERVAL:
            h_parms->sat_check_ttinsert_limit =
               CLStateGetIntArgCheckRange(handle, arg, 1, LONG_MAX);
            break;
      case OPT_SATCHECK:
            tmp = StringIndex(arg, GroundingStratNames);
            if(tmp < 0)
            {
               DStr_p err = DStrAlloc();
               DStrAppendStr(err,
                             "Wrong argument to option --sat-check. Possible "
                             "values: ");
               DStrAppendStrArray(err, GroundingStratNames, ", ");
               Error(DStrView(err), USAGE_ERROR);
               DStrFree(err);
            }
            h_parms->sat_check_grounding = tmp;
            break;
      case OPT_SAT_NORMCONST:
            h_parms->sat_check_normconst = true;
            break;
      case OPT_SAT_NORMALIZE:
            h_parms->sat_check_normalize = true;
            break;
      case OPT_SAT_DEC_LIMIT:
            h_parms->sat_check_decision_limit =
               CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_STATIC_WATCHLIST:
            h_parms->watchlist_is_static = true;
            //intentional fall-through
      case OPT_WATCHLIST:
            if(strcmp(WATCHLIST_INLINE_STRING, arg)==0 ||
               strcmp(WATCHLIST_INLINE_QSTRING, arg)==0  )
            {
               watchlist_filename = UseInlinedWatchList;
            }
            else
            {
               watchlist_filename = arg;
            }
            break;
      case OPT_WATCHLIST_NO_SIMPLIFY:
            h_parms->watchlist_simplify = false;
            break;
      case OPT_FW_SUMBSUMPTION_AGGRESSIVE:
            h_parms->forward_subsumption_aggressive = true;
            break;
      case OPT_NO_INDEXED_SUBSUMPTION:
            fvi_parms->cspec.features = FVINoFeatures;
            break;
      case OPT_FVINDEX_STYLE:
            if(strcmp(arg, "None")==0)
            {
               fvi_parms->cspec.features = FVINoFeatures;
            }
            else if(strcmp(arg, "Direct")==0)
            {
               fvi_parms->use_perm_vectors = false;
            }
            else if(strcmp(arg, "Perm")==0)
            {
               fvi_parms->use_perm_vectors = true;
               fvi_parms->eliminate_uninformative = false;
            }
            else if(strcmp(arg, "PermOpt")==0)
            {
               fvi_parms->use_perm_vectors = true;
               fvi_parms->eliminate_uninformative = true;
            }
            else
            {
               Error("Option --subsumption-indexing requires "
                     "'None', 'Direct', 'Perm', or 'PermOpt'.", USAGE_ERROR);
            }
            break;
      case OPT_FVINDEX_FEATURETYPES:
            if(strcmp(arg, "None")==0)
            {
               fvi_parms->cspec.features = FVINoFeatures;
            }
            else if(strcmp(arg, "AC")==0)
            {
               fvi_parms->cspec.features = FVIACFeatures;
            }
            else if(strcmp(arg, "SS")==0)
            {
               fvi_parms->cspec.features = FVISSFeatures;
            }
            else if(strcmp(arg, "All")==0)
            {
               fvi_parms->cspec.features = FVIAllFeatures;
            }
            else if(strcmp(arg, "Bill")==0)
            {
               fvi_parms->cspec.features = FVIBillFeatures;
            }
            else if(strcmp(arg, "BillPlus")==0)
            {
               fvi_parms->cspec.features = FVIBillPlusFeatures;
            }
            else if(strcmp(arg, "ACFold")==0)
            {
               fvi_parms->cspec.features = FVIACFold;
            }
            else if(strcmp(arg, "ACStagger")==0)
            {
               fvi_parms->cspec.features = FVIACStagger;
            }
            else
            {
               Error("Option --fvindex-featuretypes requires "
                     "'None', 'AC', 'SS', 'All', 'Bill', 'BillPlus',"
                     " 'ACFold', 'ACStagger'.", USAGE_ERROR);
            }
            break;
      case OPT_FVINDEX_MAXFEATURES:
            tmp = CLStateGetIntArg(handle, arg);
            if(tmp<=0)
            {
               Error("Argument to option --fvindex-maxfeatures "
                     "has to be > 0", USAGE_ERROR);
            }
            fvi_parms->max_symbols = CLStateGetIntArgCheckRange(handle, arg, 0, LONG_MAX);
            break;
      case OPT_FVINDEX_SLACK:
            fvi_parms->symbol_slack = CLStateGetIntArgCheckRange(handle, arg, 0, LONG_MAX);
            break;
      case OPT_RW_BW_INDEX:
            check_fp_index_arg(arg, "--rw-bw-index");
            strcpy(h_parms->rw_bw_index_type, arg);
            break;
      case OPT_PM_FROM_INDEX:
            check_fp_index_arg(arg, "--pm-from-index");
            strcpy(h_parms->pm_from_index_type, arg);
            break;
      case OPT_PM_INTO_INDEX:
            check_fp_index_arg(arg, "--pm-into-index");
            strcpy(h_parms->pm_into_index_type, arg);
            break;
      case OPT_FP_INDEX:
            check_fp_index_arg(arg, "--fp-index");
            strcpy(h_parms->rw_bw_index_type, arg);
            strcpy(h_parms->pm_from_index_type, arg);
            strcpy(h_parms->pm_into_index_type, arg);
            break;
      case OPT_PDT_NO_SIZECONSTR:
            PDTreeUseSizeConstraints = false;
            break;
      case OPT_PDT_NO_AGECONSTR:
            PDTreeUseAgeConstraints = false;
            break;
      case OPT_DETSORT_RW:
            h_parms->detsort_bw_rw = true;
            break;
      case OPT_DETSORT_NEW:
            h_parms->detsort_tmpset = true;
            break;
      case OPT_DEFINE_WFUN:
            PStackPushP(wfcb_definitions, arg);
            break;
      case OPT_DEFINE_HEURISTIC:
            /* Note that we postprocess this at the end */
            PStackPushP(hcb_definitions, arg);
            break;
      case OPT_FREE_NUMBERS:
            free_symb_prop = free_symb_prop|FPIsInteger|FPIsRational|FPIsFloat;
            break;
      case OPT_FREE_OBJECTS:
            free_symb_prop = free_symb_prop|FPIsObject;
            break;
      case OPT_DEF_CNF:
            h_parms->formula_def_limit =
               CLStateGetIntArgCheckRange(handle, arg, 0, LONG_MAX);
            break;
      case OPT_FOOL_UNROLL:
            h_parms->fool_unroll = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_MINISCOPE_LIMIT:
            h_parms->miniscope_limit =
               CLStateGetIntArgCheckRange(handle, arg, 0, LONG_MAX);
            break;
      case OPT_PRINT_TYPES:
            TermPrintTypes = true;
            break;
      case OPT_APP_ENCODE:
            app_encode = true;
            break;
      case OPT_ARG_CONG:
            if(!strcmp(arg, "all"))
            {
               h_parms->arg_cong = AllLits;
            } else if (!strcmp(arg, "max"))
            {
               h_parms->arg_cong = MaxLits;
            } else if (!strcmp(arg, "off"))
            {
               h_parms->arg_cong = NoLits;
            } else
            {
               Error("neg-ext excepts either all, max or off", 0);
            }
            break;
      case OPT_NEG_EXT:
            if(!strcmp(arg, "all"))
            {
               h_parms->neg_ext = AllLits;
            }
            else if (!strcmp(arg, "max"))
            {
               h_parms->neg_ext = MaxLits;
            }
            else if (!strcmp(arg, "off"))
            {
               h_parms->neg_ext = NoLits;
            }
            else
            {
               Error("neg-ext excepts either all or max", 0);
            }
            break;
      case OPT_POS_EXT:
            if(!strcmp(arg, "all"))
            {
               h_parms->pos_ext = AllLits;
            }
            else if (!strcmp(arg, "max"))
            {
               h_parms->pos_ext = MaxLits;
            }
            else if (!strcmp(arg, "off"))
            {
               h_parms->pos_ext = NoLits;
            }
            else
            {
               Error("pos-ext excepts either all or max", 0);
            }
            break;
      case OPT_FUNC_PROJ_IMIT:
            h_parms->func_proj_limit = CLStateGetIntArgCheckRange(handle, arg, 0, 63);
            break;
      case OPT_IMIT_LIMIT:
            h_parms->imit_limit = CLStateGetIntArgCheckRange(handle, arg, 0, 63);
            break;
      case OPT_IDENT_LIMIT:
            h_parms->ident_limit = CLStateGetIntArgCheckRange(handle, arg, 0, 63);
            break;
      case OPT_ELIM_LIMIT:
            h_parms->elim_limit = CLStateGetIntArgCheckRange(handle, arg, 0, 63);
            break;
      case OPT_MAX_UNIFIERS:
            h_parms->max_unifiers = CLStateGetIntArgCheckRange(handle, arg, 0, 1024);
            break;
      case OPT_MAX_UNIF_STEPS:
            h_parms->max_unif_steps = CLStateGetIntArgCheckRange(handle, arg, 0, 100000);
            break;
      case OPT_UNIF_MODE:
            unif_mode = STR2UM(arg);
            if(unif_mode==-1)
            {
               Error("values of unif mode are eiter single or multi", 0);
            }
            h_parms->unif_mode = unif_mode;
            break;
      case OPT_PATTERN_ORACLE:
            h_parms->pattern_oracle = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_FIXPOINT_ORACLE:
            h_parms->fixpoint_oracle = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_EXT_SUP:
            h_parms->ext_rules_max_depth =
               CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_INVERSE_RECOGNITION:
            h_parms->inverse_recognition = true;
            break;
      case OPT_REPLACE_INJ_DEFS:
            h_parms->replace_inj_defs = true;
            break;
      case OPT_BCE:
            h_parms->bce = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_BCE_MAX_OCCS:
            h_parms->bce_max_occs = CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_PRED_ELIM:
            h_parms->pred_elim = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_PRED_ELIM_GATES:
            h_parms->pred_elim_gates = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_PRED_ELIM_MAX_OCCS:
            h_parms->pred_elim_max_occs = CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_PRED_ELIM_TOLERANCE:
            h_parms->pred_elim_tolerance = CLStateGetIntArgCheckRange(handle, arg, 0, INT_MAX);
            break;
      case OPT_PRED_ELIM_FORCE_MU_DECREASE:
            h_parms->pred_elim_force_mu_decrease = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_PRED_ELIM_IGNORE_CONJ_SYMS:
            h_parms->pred_elim_ignore_conj_syms = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_LAMBDA_TO_FORALL:
            h_parms->lambda_to_forall = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_ELIM_LEIBNIZ:
            h_parms->elim_leibniz_max_depth = CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_UNROLL_FORMULAS_ONLY:
            h_parms->unroll_only_formulas = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_PRIM_ENUM_MAX_DEPTH:
            h_parms->prim_enum_max_depth = CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_PRUNE_ARGS:
            h_parms->prune_args = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_CHOICE_INST:
            h_parms->inst_choice_max_depth = CLStateGetIntArgCheckRange(handle, arg, -1, INT_MAX);
            break;
      case OPT_LOCAL_RW:
            h_parms->local_rw = CLStateGetBoolArg(handle, arg);
            break;
      case OPT_PRIM_ENUM_KIND:
            if(strcmp(arg, "neg")==0)
            {
               h_parms->prim_enum_mode = NegMode;
            }
            else if(strcmp(arg, "and")==0)
            {
               h_parms->prim_enum_mode = AndMode;
            }
            else if(strcmp(arg, "or")==0)
            {
               h_parms->prim_enum_mode = OrMode;
            }
            else if(strcmp(arg, "eq")==0)
            {
               h_parms->prim_enum_mode = EqMode;
            }
            else if(strcmp(arg, "pragmatic")==0)
            {
               h_parms->prim_enum_mode = PragmaticMode;
            }
            else if(strcmp(arg, "full")==0)
            {
               h_parms->prim_enum_mode = FullMode;
            }
            else if(strcmp(arg, "logsym")==0)
            {
               h_parms->prim_enum_mode = LogSymbolMode;
            }
            else
            {
               Error("Option --prim-enum-mode excepts neg, and, or, eq, pragmatic, full, or logsym",
                     USAGE_ERROR);
            }
            break;
      case OPT_ETA_NORMALIZE:
            if(strcmp(arg, "reduce")==0)
            {
               SetEtaNormalizer(LambdaEtaReduceDB);
            }
            else if(strcmp(arg, "expand")==0)
            {
               SetEtaNormalizer(LambdaEtaExpandDB);
            }
            else
            {
               Error("Option --eta-normalize requires 'reduce' or 'expand' as an argument",
                     USAGE_ERROR);
            }
            break;
      case OPT_HO_ORDER_KIND:
            if(strcmp(arg, "lfho")==0)
            {
               h_parms->order_params.ho_order_kind = LFHO_ORDER;
            }
            else if(strcmp(arg, "lambda")==0)
            {
               h_parms->order_params.ho_order_kind = LAMBDA_ORDER;
            }
            else
            {
               Error("Option --ho-order-kind requires 'lfho' or 'lambda' as an argument",
                     USAGE_ERROR);
            }
            break;
      case OPT_CNF_TIMEOUT_PORTION:
            clausification_time_part =
               CLStateGetIntArgCheckRange(handle, arg, 1, 99) / (double) 100;
      case OPT_PREINSTANTIATE_INDUCTION:
            h_parms->preinstantiate_induction = CLStateGetBoolArg(handle, arg);
            break;
      default:
            assert(false && "Unknown option");
            break;
      }
   }
   if(num_cpus == -1)
   {
      num_cpus = GetCoreNumber();
   }
   if(!PStackEmpty(hcb_definitions))
   {
      h_parms->heuristic_def = PStackTopP(hcb_definitions);
   }

   if(!strategy_scheduling)
   {
      set_limits(HardTimeLimit, SoftTimeLimit, h_parms->mem_limit);
   }

   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n\
E " VERSION " \"" E_NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
Read a set of first-order (or, in the -ho-version, higher-order)\n\
clauses and formulae and try to prove the conjecture (if given)\n\
or show the set unsatisfiable.\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
