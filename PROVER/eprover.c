/*-----------------------------------------------------------------------

  File  : eprover.c

  Author: Stephan Schulz

  Contents

  Main program for the E equational theorem prover.

  Copyright 1998-2018 by the authors.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Tue Jun  9 01:32:15 MET DST 1998 - New.

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
HeuristicParms_p  h_parms;
FVIndexParms_p    fvi_parms;
bool              print_sat = false,
   print_full_deriv = false,
   print_statistics = false,
   filter_sat = false,
   print_rusage = false,
   print_pid = false,
   print_version = false,
   outinfo = false,
   error_on_empty = false,
   no_preproc = false,
   no_eq_unfold = false,
   pcl_full_terms = true,
   indexed_subsumption = true,
   prune_only = false,
   new_cnf = true,
   cnf_only = false,
   inf_sys_complete = true,
   assume_inf_sys_complete = false,
   incomplete = false,
   conjectures_are_questions = false,
   app_encode = false,
   strategy_scheduling = false;
ProofOutput       print_derivation = PONone;
long              proc_training_data;

IOFormat          parse_format = AutoFormat;
long              step_limit = LONG_MAX,
   answer_limit = 1,
   proc_limit = LONG_MAX,
   unproc_limit = LONG_MAX,
   total_limit = LONG_MAX,
   generated_limit = LONG_MAX,
   eqdef_maxclauses = DEFAULT_EQDEF_MAXCLAUSES,
   relevance_prune_level = 0,
   miniscope_limit = 1048576;
long long tb_insert_limit = LLONG_MAX;

int eqdef_incrlimit = DEFAULT_EQDEF_INCRLIMIT,
   force_deriv_output = 0;
char              *outdesc = DEFAULT_OUTPUT_DESCRIPTOR,
   *filterdesc = DEFAULT_FILTER_DESCRIPTOR;
PStack_p          wfcb_definitions, hcb_definitions;
char              *sine=NULL;
pid_t              pid = 0;

FunctionProperties free_symb_prop = FPIgnoreProps;

ProblemType problemType  = PROBLEM_NOT_INIT;

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

   proofstate->has_interpreted_symbols =
      FormulaSetHasInterpretedSymbol(proofstate->f_axioms);
   parsed_ax_no = ProofStateAxNo(proofstate);

   if(error_on_empty_local && (parsed_ax_no == 0))
   {
#ifdef PRINT_SOMEERRORS_STDOUT
      fprintf(GlobalOut, "# Error: Input file contains no clauses or formulas\n");
      TSTPOUT(GlobalOut, "InputError");
#endif
      Error("Input file contains no clauses or formulas", OTHER_ERROR);
   }
   *ax_no = parsed_ax_no;

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
      fprintf(GlobalOut, "# Pid: %lld\n", (long long)pid);
      fflush(GlobalOut);
   }
   if(print_version)
   {
      fprintf(GlobalOut, "# Version: %s\n", VERSION);
      fflush(GlobalOut);
   }
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
   if(OutputLevel||print_statistics)
   {
      fprintf(GlobalOut, "# Parsed axioms                        : %ld\n",
              parsed_ax_no);
      fprintf(GlobalOut, "# Removed by relevancy pruning/SinE    : %ld\n",
              relevancy_pruned);
      fprintf(GlobalOut, "# Initial clauses                      : %ld\n",
              raw_clause_no);
      fprintf(GlobalOut, "# Removed in clause preprocessing      : %ld\n",
              preproc_removed);
      ProofStateStatisticsPrint(GlobalOut, proofstate);
      fprintf(GlobalOut, "# Clause-clause subsumption calls (NU) : %ld\n",
              ClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, "# Rec. Clause-clause subsumption calls : %ld\n",
              ClauseClauseSubsumptionCallsRec);
      fprintf(GlobalOut, "# Non-unit clause-clause subsumptions  : %ld\n",
              ClauseClauseSubsumptionSuccesses);
      fprintf(GlobalOut, "# Unit Clause-clause subsumption calls : %ld\n",
              UnitClauseClauseSubsumptionCalls);
      fprintf(GlobalOut, "# Rewrite failures with RHS unbound    : %ld\n",
              RewriteUnboundVarFails);
      fprintf(GlobalOut, "# BW rewrite match attempts            : %ld\n",
              BWRWMatchAttempts);
      fprintf(GlobalOut, "# BW rewrite match successes           : %ld\n",
              BWRWMatchSuccesses);
      fprintf(GlobalOut, "# Condensation attempts                : %ld\n",
              CondensationAttempts);
      fprintf(GlobalOut, "# Condensation successes               : %ld\n",
              CondensationSuccesses);

#ifdef MEASURE_UNIFICATION
      fprintf(GlobalOut, "# Unification attempts                 : %ld\n",
              UnifAttempts);
      fprintf(GlobalOut, "# Unification successes                : %ld\n",
              UnifSuccesses);
#endif
#ifdef PDT_COUNT_NODES
      fprintf(GlobalOut, "# PDT nodes visited                    : %ld\n",
              PDTNodeCounter);
#endif
      fprintf(GlobalOut, "# Termbank termtop insertions          : %lld\n",
              proofstate->terms->insertions);
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
      fprintf(GlobalOut, "# Backwards rewriting index : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.bw_rw_index);
      fprintf(GlobalOut, "\n");
      /*FPIndexPrintDot(GlobalOut, "rw_bw_index",
        proofstate->gindices.bw_rw_index,
        SubtermTreePrintDot,
        proofstate->signature);*/
      fprintf(GlobalOut, "# Paramod-from index        : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_from_index);
      fprintf(GlobalOut, "\n");
      FPIndexPrintDot(GlobalOut, "pm_from_index",
                      proofstate->gindices.pm_from_index,
                      SubtermTreePrintDot,
                      proofstate->signature);
      fprintf(GlobalOut, "# Paramod-into index        : ");
      FPIndexDistribDataPrint(GlobalOut, proofstate->gindices.pm_into_index);
      fprintf(GlobalOut, "\n");
      fprintf(GlobalOut, "# Paramod-neg-atom index    : ");
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
//   Main entry point of the prover.
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
   Derivation_p deriv;

   assert(argv[0]);

#ifdef STACK_SIZE
   INCREASE_STACK_SIZE;
#endif

   pid = getpid();
   InitIO(NAME);

   ESignalSetup(SIGXCPU);

   h_parms = HeuristicParmsAlloc();
   fvi_parms = FVIndexParmsAlloc();
   wfcb_definitions = PStackAlloc();
   hcb_definitions = PStackAlloc();

   state = process_options(argc, argv);

   OpenGlobalOut(outname);
   print_info();


   if(state->argc ==  0)
   {
      CLStateInsertArg(state, "-");
   }

   proofstate = parse_spec(state, parse_format,
                           error_on_empty, free_symb_prop,
                           &parsed_ax_no);

   relevancy_pruned += ProofStateSinE(proofstate, sine);
   relevancy_pruned += ProofStatePreprocess(proofstate, relevance_prune_level);

   if(app_encode)
   {
      FormulaSetAppEncode(stdout, proofstate->f_axioms);
      goto cleanup1;
   }

   if(strategy_scheduling)
   {
      ExecuteSchedule(StratSchedule, h_parms, print_rusage);
   }

   FormulaSetDocInital(GlobalOut, OutputLevel, proofstate->f_axioms);
   ClauseSetDocInital(GlobalOut, OutputLevel, proofstate->axioms);

   if(prune_only)
   {
      fprintf(GlobalOut, "\n# Pruning successful!\n");
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

   if(new_cnf)
   {
      cnf_size = FormulaSetCNF2(proofstate->f_axioms,
                                proofstate->f_ax_archive,
                                proofstate->axioms,
                                proofstate->terms,
                                proofstate->freshvars,
                                proofstate->gc_terms,
                                miniscope_limit);
   }
   else
   {
      cnf_size = FormulaSetCNF(proofstate->f_axioms,
                               proofstate->f_ax_archive,
                               proofstate->axioms,
                               proofstate->terms,
                               proofstate->freshvars,
                               proofstate->gc_terms);
   }

   if(cnf_size)
   {
      VERBOUT("CNFization done\n");
   }

   raw_clause_no = proofstate->axioms->members;
   ProofStateLoadWatchlist(proofstate, watchlist_filename, parse_format);

   if(!no_preproc)
   {
      ClauseSetArchiveCopy(proofstate->ax_archive, proofstate->axioms);
      if(proofstate->watchlist)
      {
         ClauseSetArchiveCopy(proofstate->ax_archive, proofstate->watchlist);
      }
      preproc_removed = ClauseSetPreprocess(proofstate->axioms,
                                            proofstate->watchlist,
                                            proofstate->archive,
                                            proofstate->tmp_terms,
                                            eqdef_incrlimit,
                                            eqdef_maxclauses);
   }

   proofcontrol = ProofControlAlloc();
   ProofControlInit(proofstate, proofcontrol, h_parms,
                    fvi_parms, wfcb_definitions, hcb_definitions);
   PCLFullTerms = pcl_full_terms; /* Preprocessing always uses full
                                     terms, so we set the flag for
                                     the main proof search only now! */

   GlobalIndicesInit(&(proofstate->wlindices),
                     proofstate->signature,
                     proofcontrol->heuristic_parms.rw_bw_index_type,
                     "NoIndex",
                     "NoIndex");
   //printf("Alive (1)!\n");

   ProofStateInit(proofstate, proofcontrol);
   
   //printf("Alive (2)!\n");
   //ProofStateInitWatchlist(proofstate, proofcontrol->ocb);


   VERBOUT2("Prover state initialized\n");
   preproc_time = GetTotalCPUTime();
   if(print_rusage)
   {
      fprintf(GlobalOut, "# Preprocessing time       : %.3f s\n", preproc_time);
   }
   if(proofcontrol->heuristic_parms.presat_interreduction)
   {
      LiteralSelectionFun sel_strat =
         proofcontrol->heuristic_parms.selection_strategy;

      proofcontrol->heuristic_parms.selection_strategy = SelectNoGeneration;
      success = Saturate(proofstate, proofcontrol, LONG_MAX,
                         LONG_MAX, LONG_MAX, LONG_MAX, LONG_MAX,
                         LLONG_MAX, LONG_MAX);
      fprintf(GlobalOut, "# Presaturation interreduction done\n");
      proofcontrol->heuristic_parms.selection_strategy = sel_strat;
      if(!success)
      {
         ProofStateResetProcessed(proofstate, proofcontrol);
      }
   }
   PERF_CTR_ENTRY(SatTimer);

#ifdef ENABLE_LFHO
   // if the problem is HO -> we have to use KBO6
   assert(problemType != PROBLEM_HO || proofcontrol->ocb->type == KBO6);
#endif

   if(!success)
   {
      success = Saturate(proofstate, proofcontrol, step_limit,
                         proc_limit, unproc_limit, total_limit,
                         generated_limit, tb_insert_limit, answer_limit);
   }
   PERF_CTR_EXIT(SatTimer);

   if(SigHasUnimplementedInterpretedSymbols(proofstate->signature))
   {
      inf_sys_complete = false;
   }

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
      fprintf(GlobalOut, "\n# Proof found!\n");

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
                                    OutputLevel||print_statistics);
         ProofStateAnalyseGC(proofstate);
         ProofStateTrain(proofstate, proc_training_data&TSPrintPos,
                         proc_training_data&TSPrintNeg);
      }
      DerivationFree(deriv);
   }
   else if(proofstate->watchlist && ClauseSetEmpty(proofstate->watchlist))
   {
      ProofStatePropDocQuote(GlobalOut, OutputLevel,
                             CPSubsumesWatch, proofstate,
                             "final_subsumes_wl");
      fprintf(GlobalOut, "\n# Watchlist is empty!\n");
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
         fprintf(GlobalOut, "\n# CNFization successful!\n");
         TSTPOUT(GlobalOut, "Unknown");
      }
      else if(out_of_clauses)
      {
         if(!(inf_sys_complete || assume_inf_sys_complete))
         {
            fprintf(GlobalOut,
                    "\n# Clause set closed under "
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
                    "\n# Clause set saturated up to interpreted theories!\n");
            if(!SilentTimeOut)
            {
               TSTPOUT(GlobalOut, "GaveUp");
            }
            retval = INCOMPLETE_PROOFSTATE;
         }
         else if(proofstate->state_is_complete && inf_sys_complete)
         {
            fprintf(GlobalOut, "\n# No proof found!\n");
            TSTPOUT(GlobalOut, neg_conjectures?"CounterSatisfiable":"Satisfiable");
            sat_status = "Saturation";
            retval = SATISFIABLE;
         }
         else
         {
            fprintf(GlobalOut, "\n# Failure: Out of unprocessed clauses!\n");
            if(!SilentTimeOut)
            {
               TSTPOUT(GlobalOut, "GaveUp");
            }
            retval = INCOMPLETE_PROOFSTATE;
         }
      }
      else
      {
         fprintf(GlobalOut, "\n# Failure: User resource limit exceeded!\n");
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
                                   OutputLevel||print_statistics);
      }

   }
   /* ClauseSetDerivationStackStatistics(proofstate->unprocessed); */
   if(print_sat)
   {
      if(proofstate->non_redundant_deleted)
      {
         fprintf(GlobalOut, "\n# Saturated system is incomplete!\n");
      }
      if(success)
      {
         fprintf(GlobalOut, "# Saturated system contains the empty clause:\n");
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
           "# sizeof TermCell     : %ld\n"
           "# sizeof EqnCell      : %ld\n"
           "# sizeof ClauseCell   : %ld\n"
           "# sizeof PTreeCell    : %ld\n"
           "# sizeof PDTNodeCell  : %ld\n"
           "# sizeof EvalCell     : %ld\n"
           "# sizeof ClausePosCell: %ld\n"
           "# sizeof PDArrayCell  : %ld\n",
           sizeof(TermCell),
           sizeof(EqnCell),
           sizeof(ClauseCell),
           sizeof(PTreeCell),
           sizeof(PDTNodeCell),
           sizeof(EvalCell),
           sizeof(ClausePosCell),
           sizeof(PDArrayCell));
   fprintf(GlobalOut, "# Estimated memory usage: %ld\n",
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
//   Since the watchlist unit clause index allows for an =auto argument
//   this is also checked explicitly here.
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
   else if((strcmp(opt, "--watchlist-unit-clause-index")==0)
            &&(strcmp(arg, "auto")==0))
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
   DStrAppendStr(err,
                 ". The watchlist unit clause index also allows the option: auto.");
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
      case OPT_PRUNE_ONLY:
            OutputLevel = 4;
            prune_only   = true;
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
            h_parms->heuristic_name = "Auto";
            h_parms->order_params.ordertype = AUTO;
            sine = "Auto";
            break;
      case OPT_SATAUTO:
            h_parms->heuristic_name = "Auto";
            h_parms->order_params.ordertype = AUTO;
            break;
      case OPT_AUTODEV:
            h_parms->heuristic_name = "AutoDev";
            h_parms->order_params.ordertype = AUTODEV;
            sine = "Auto";
            break;
      case OPT_SATAUTODEV:
            h_parms->heuristic_name = "AutoDev";
            h_parms->order_params.ordertype = AUTODEV;
            break;
      case OPT_AUTO_SCHED:
            strategy_scheduling = true;
            sine = "Auto";
            break;
      case OPT_SATAUTO_SCHED:
            strategy_scheduling = true;
            break;
      case OPT_NO_PREPROCESSING:
            no_preproc = true;
            break;
      case OPT_EQ_UNFOLD_LIMIT:
            eqdef_incrlimit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_EQ_UNFOLD_MAXCLAUSES:
            eqdef_maxclauses = CLStateGetIntArg(handle, arg);
            break;
      case OPT_NO_EQ_UNFOLD:
            eqdef_incrlimit = INT_MIN;
            break;
      case OPT_SINE:
            sine = arg;
            break;
      case OPT_REL_PRUNE_LEVEL:
            relevance_prune_level = CLStateGetIntArg(handle, arg);
            break;
      case OPT_PRESAT_SIMPLIY:
            h_parms->presat_interreduction = true;
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
            if(h_parms->selection_strategy == SelectNoGeneration)
            {
               inf_sys_complete = false;
            }
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
      case OPT_ORDERING:
            if(strcmp(arg, "Auto")==0)
            {
               h_parms->order_params.ordertype = AUTO;
            }
            else if(strcmp(arg, "AutoCASC")==0)
            {
               h_parms->order_params.ordertype = AUTOCASC;
            }
            else if(strcmp(arg, "AutoDev")==0)
            {
               h_parms->order_params.ordertype = AUTODEV;
            }
            else if(strcmp(arg, "AutoSched0")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED0;
            }
            else if(strcmp(arg, "AutoSched1")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED1;
            }
            else if(strcmp(arg, "AutoSched2")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED2;
            }
            else if(strcmp(arg, "AutoSched3")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED3;
            }
            else if(strcmp(arg, "AutoSched4")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED4;
            }
            else if(strcmp(arg, "AutoSched5")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED5;
            }
            else if(strcmp(arg, "AutoSched6")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED6;
            }
            else if(strcmp(arg, "AutoSched7")==0)
            {
               h_parms->order_params.ordertype = AUTOSCHED7;
            }
            else if(strcmp(arg, "Optimize")==0)
            {
               h_parms->order_params.ordertype = OPTIMIZE_AX;
            }
            else if(strcmp(arg, "LPO")==0)
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
            else
            {
               Error("Option -t (--term-ordering) requires Auto, "
                     "AutoCASC, AutoDev, AutoSched0, AutoSched1, "
                     "AutoSched2, AutoSched3, AutoSched4, AutoSched5,"
                     "AutoSched6, AutoSched7, Optimize, "
                     "LPO, LPO4, KBO or KBO6 as an argument",
                     USAGE_ERROR);
            }
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
            if(tmp <= 0)
            {
               DStr_p err = DStrAlloc();
               DStrAppendStr(err,
                             "Wrong argument to option --sat-check. Possible "
                             "values: ");
               DStrAppendStrArray(err, GroundingStratNames+1, ", ");
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
      case OPT_WATCHLIST_CLAUSE_ABSTRACTION:
            if (!strcmp("constants", arg))
            {
               h_parms->wl_abstract_skolem_sym = false;
               h_parms->wl_abstract_constant_sym = true;
            }
            else if (!strcmp("skolem", arg))
            {
               h_parms->wl_abstract_skolem_sym = true;
               h_parms->wl_abstract_constant_sym = false;
            }
            else
            {
               printf("Please either use constants or skolem as an argument.");
            }
            break;
      case OPT_WATCHLIST_UNIT_CLAUSE_INDEX:
            check_fp_index_arg(arg, "--watchlist-unit-clause-index");
            strcpy(h_parms->watchlist_unit_clause_index_type, arg);
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
            PStackPushP(hcb_definitions, arg);
            break;
      case OPT_FREE_NUMBERS:
            free_symb_prop = free_symb_prop|FPIsInteger|FPIsRational|FPIsFloat;
            break;
      case OPT_FREE_OBJECTS:
            free_symb_prop = free_symb_prop|FPIsObject;
            break;
      case OPT_DEF_CNF_OLD:
            new_cnf = false;
            /* Intentional fall-through */
      case OPT_DEF_CNF:
            FormulaDefLimit = CLStateGetIntArgCheckRange(handle, arg, 0, LONG_MAX);
            break;
      case OPT_MINISCOPE_LIMIT:
            miniscope_limit =  CLStateGetIntArgCheckRange(handle, arg, 0, LONG_MAX);
            break;
      case OPT_PRINT_TYPES:
            TermPrintTypes = true;
            break;
      case OPT_APP_ENCODE:
            app_encode = true;
            break;
      default:
            assert(false && "Unknown option");
            break;
      }
   }
   if((HardTimeLimit!=RLIM_INFINITY)||(SoftTimeLimit!=RLIM_INFINITY))
   {
      if(SoftTimeLimit!=RLIM_INFINITY)
      {
         SetSoftRlimitErr(RLIMIT_CPU, SoftTimeLimit, "RLIMIT_CPU (E-Soft)");
         TimeLimitIsSoft = true;
      }
      else
      {
         SetSoftRlimitErr(RLIMIT_CPU, HardTimeLimit, "RLIMIT_CPU (E-Hard)");
         TimeLimitIsSoft = false;
      }

      if(SetSoftRlimit(RLIMIT_CORE, 0)!=RLimSuccess)
      {
         perror("eprover");
         Warning("Cannot prevent core dumps!");
      }
   }
   SetMemoryLimit(h_parms->mem_limit);

   return state;
}

void print_help(FILE* out)
{
   fprintf(out, "\n\
E " VERSION " \"" E_NICKNAME "\"\n\
\n\
Usage: " NAME " [options] [files]\n\
\n\
Read a set of first-order clauses and formulae and try to refute it.\n\
\n");
   PrintOptions(stdout, opts, "Options:\n\n");
   fprintf(out, "\n\n" E_FOOTER);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
