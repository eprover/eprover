/*-----------------------------------------------------------------------

  File  : eprover.c

  Author: Stephan Schulz

  Contents

  Main program for the E equational theorem prover.

  Copyright 1998-2017 by the authors.
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
#include <cco_scheduling.h>
#include <e_version.h>
#include <ccl_rewrite.h>
#include <ccl_pdtrees.h>


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
   miniscope_limit = 1000;
long long tb_insert_limit = LLONG_MAX;

int eqdef_incrlimit = DEFAULT_EQDEF_INCRLIMIT,
   force_deriv_output = 0;
char              *outdesc = DEFAULT_OUTPUT_DESCRIPTOR,
   *filterdesc = DEFAULT_FILTER_DESCRIPTOR;
PStack_p          wfcb_definitions, hcb_definitions;
char              *sine=NULL;
pid_t              pid = 0;
int               ProblemIsHO = PROBLEM_NOT_INIT;

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
   Scanner_p in = NULL;
   int i;
   StrTree_p skip_includes = NULL;
   long parsed_ax_no;

   proofstate = ProofStateAlloc(free_symb_prop_local);
   for(i=0; state->argv[i]; i++)
   {
      in = CreateScanner(StreamTypeFile, state->argv[i], true, NULL);
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

      FormulaAndClauseSetParse(in, proofstate->axioms,
                               proofstate->f_axioms,
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

   // /* TODO: Remove this! */
   // if (in && ScannerQuerySupportHO(in))
   // {
   //    TBPrintBankInOrder(stderr, proofstate->terms);
   // }

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
      fprintf(GlobalOut, "# Version: " VERSION "\n");
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

void test_pdts(ProofState_p proofstate)
{
  /* The file is lfho.pdt.p */

   /* Assuming this state of clauses:
    1 tcf(i_0_10, plain, ![X2:$i > $i]:g($@_var(X2,a),c)=d).
    2 tcf(i_0_15, negated_conjecture, ![X8:$i > $i > $i]:$@_var(X8,a,c)=f(c,d,a,b)).
    3 tcf(i_0_8, plain, ![X1:$i > $i > $i]:$@_var(X1,a,c)=f(c,d,a,b)).
    4 tcf(i_0_12, plain, ![X5:$i > $i > $i > $i > $i, X4, X3:$i > $i > $i > $i]:$@_var(X3,a,X4,b)=$@_var(X5,b,a,c,b)).
    5 cnf(i_0_14, plain, (f(X7,b,X7,d)=f(a,b,c,d))).
    6 cnf(i_0_11, plain, (g(h(b,a),c)=g(f(a,a,a,a),b))).
    7 tcf(i_0_13, plain, ![X6:$i > $i > $i]:$@_var(X6,a)!=f(c,a,d)).
    8 tcf(i_0_9, plain, f(c,d,a)!=f(a,a,a)).

  */

   /* appliedVariable -- clause 3 -- X a c*/
   ClausePos_p appVar = ClausePosCellAlloc();
   Clause_p    appVarClause = proofstate->axioms->anchor->succ->succ->succ;
   appVar->clause  = appVarClause;
   appVar->literal = appVarClause->literals;

   /*Just a hack to try rewriting */
   EqnSetProp(appVar->literal, EPIsOriented);
   appVarClause->date = 1;

   appVar->side    = LeftSide;
   appVar->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, appVar);

   /* partially applied term -- clause 8  -- f c d a*/
   ClausePos_p partiallyApplied = ClausePosCellAlloc();
   Clause_p    partiallyAppliedClause = proofstate->axioms->anchor->succ->succ->succ->succ
                                                                  ->succ->succ->succ->succ;
   partiallyApplied->clause  = partiallyAppliedClause;
   partiallyApplied->literal = partiallyAppliedClause->literals;

   EqnSetProp(partiallyApplied->literal, EPIsOriented);
   partiallyAppliedClause->date = 1;

   partiallyApplied->side    = LeftSide;
   partiallyApplied->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, partiallyApplied);

   /* first term to match -- clause 2, right -- f c d a b*/
   Term_p firstToMatch = proofstate->axioms->anchor->succ->succ->literals->rterm;

   PDTreeSearchInit(proofstate->processed_pos_rules->demod_index, firstToMatch, SysDateInvalidTime(), true);
   Subst_p subst = SubstAlloc();
   MatchInfo_p mi = PDTreeFindNextDemodulator(proofstate->processed_pos_rules->demod_index, subst);

   fprintf(stderr, "Matching :");
   TermPrint(stderr, firstToMatch, proofstate->terms->sig, DEREF_ALWAYS);
   fprintf(stderr, ".\n");

   Term_p matcher =  GetMatcher(mi);
   if (!matcher)
   {
      fprintf(stderr, "Nothing found (%p)!!!\n", mi);
   }
   else
   {
      fprintf(stderr, "Matcher :");
      TermPrint(stderr, matcher, proofstate->terms->sig, DEREF_ALWAYS);
      fprintf(stderr, "\nWhat is matched : ");
      TermPrint(stderr, MatchInfoMatchedPrefix(mi, firstToMatch), proofstate->terms->sig, DEREF_ALWAYS);
   }
   SubstBacktrack(subst);
   PDTreeSearchExit(proofstate->processed_pos_rules->demod_index);

   /* applied var inside -- clause 1, left -- g (X a) c*/
   ClausePos_p appVarInside = ClausePosCellAlloc();
   Clause_p    appVarInsideClause = proofstate->axioms->anchor->succ;
   appVarInside->clause  = appVarInsideClause;
   appVarInside->literal = appVarInsideClause->literals;

   EqnSetProp(appVarInside->literal, EPIsOriented);
   appVarInsideClause->date = 1;

   appVarInside->side    = LeftSide;
   appVarInside->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, appVarInside);


   //PDTreePrint(stderr, proofstate->processed_pos_rules->demod_index);
   /* second to match -- clause 6, left  -- g (h b a) c*/
   Term_p secondToMatch = proofstate->axioms->anchor->succ->succ->succ->
                                                      succ->succ->succ->literals->lterm;
   fprintf(stderr, "\nSecond to match :");
   TermPrint(stderr, secondToMatch, proofstate->terms->sig, DEREF_ALWAYS);

   PDTreeSearchInit(proofstate->processed_pos_rules->demod_index, secondToMatch, SysDateInvalidTime(), true);
   mi = PDTreeFindNextDemodulator(proofstate->processed_pos_rules->demod_index, subst);

   matcher =  GetMatcher(mi);
   if (!matcher)
   {
      fprintf(stderr, "Nothing found (%p)!!!\n", mi);
   }
   else
   {
      fprintf(stderr, "\nMatcher :");
      TermPrint(stderr, matcher, proofstate->terms->sig, DEREF_ALWAYS);
      fprintf(stderr, "\nWhat is matched : ");
      TermPrint(stderr, MatchInfoMatchedPrefix(mi, secondToMatch), proofstate->terms->sig, DEREF_ALWAYS);
   }


   SubstBacktrack(subst);
   PDTreeSearchExit(proofstate->processed_pos_rules->demod_index);

   /* Varible both applied an nonaplied in the same term -- X a Y b -- 4 left*/
   ClausePos_p appVarInOut = ClausePosCellAlloc();
   Clause_p    appVarInOutClause = proofstate->axioms->anchor->succ->succ->succ->succ;
   appVarInOut->clause  = appVarInOutClause;
   appVarInOut->literal = appVarInOutClause->literals;

   EqnSetProp(appVarInOut->literal, EPIsOriented);
   appVarInOutClause->date = 1;

   appVarInOut->side    = LeftSide;
   appVarInOut->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, appVarInOut);

   fprintf(stderr, "\nCurrently in the PDT: \n");
   PDTreePrint(stderr, proofstate->processed_pos_rules->demod_index);


   Term_p thirdToMatch = proofstate->axioms->anchor->succ->succ->succ->succ->literals->rterm;
   fprintf(stderr, "\nThird to match :");
   /* X(b,a,c,b) -- 4 -- right*/
   TermPrint(stderr, thirdToMatch, proofstate->terms->sig, DEREF_ALWAYS);

   PDTreeSearchInit(proofstate->processed_pos_rules->demod_index, thirdToMatch, SysDateInvalidTime(), true);
   mi = PDTreeFindNextDemodulator(proofstate->processed_pos_rules->demod_index, subst);

   matcher =  GetMatcher(mi);
   if (!matcher)
   {
      fprintf(stderr, "\nNothing found (%p)!!!\n", mi);
   }
   else
   {
      fprintf(stderr, "\nMatcher :");
      TermPrint(stderr, matcher, proofstate->terms->sig, DEREF_NEVER);
      fprintf(stderr, "\nWhat is matched : ");
      TermPrint(stderr, MatchInfoMatchedPrefix(mi, thirdToMatch), proofstate->terms->sig, DEREF_ALWAYS);
   }


   SubstBacktrack(subst);
   PDTreeSearchExit(proofstate->processed_pos_rules->demod_index);


   /* Idea is that this variable matches up to some point -- X a -> 
      has to match f c d a b .. X matches f c d and a matches a..
      so it is matched up to some point */
   ClausePos_p appVarPar = ClausePosCellAlloc();
   Clause_p    appVarParClause = proofstate->axioms->anchor->succ->succ->succ
                                                           ->succ->succ->succ->succ;
   appVarPar->clause  = appVarParClause;
   appVarPar->literal = appVarParClause->literals;

   EqnSetProp(appVarPar->literal, EPIsOriented);
   appVarParClause->date = 1;


   appVarPar->side    = LeftSide;
   appVarPar->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, appVarPar);
   

   fprintf(stderr, "\nFourth to match :");
   TermPrint(stderr, firstToMatch, proofstate->terms->sig, DEREF_ALWAYS);
   PDTreeSearchInit(proofstate->processed_pos_rules->demod_index, firstToMatch, SysDateInvalidTime(), true);

   mi = PDTreeFindNextDemodulator(proofstate->processed_pos_rules->demod_index, subst);

   matcher =  GetMatcher(mi);
   if (!matcher)
   {
      fprintf(stderr, "\nNothing found (%p)!!!\n", mi);
   }
   else
   {
      fprintf(stderr, "\nMatcher :");
      TermPrint(stderr, matcher, proofstate->terms->sig, DEREF_ALWAYS);
      fprintf(stderr, "\nWhat is matched : ");
      TermPrint(stderr, MatchInfoMatchedPrefix(mi, firstToMatch), proofstate->terms->sig, DEREF_ALWAYS);
   }

   SubstBacktrack(subst);
   PDTreeSearchExit(proofstate->processed_pos_rules->demod_index);

   ClausePos_p sameVarTwice = ClausePosCellAlloc();
   Clause_p    sameVarTwiceClause = proofstate->axioms->anchor->succ->succ->succ
                                                              ->succ->succ;
   sameVarTwice->clause  = sameVarTwiceClause;
   sameVarTwice->literal = sameVarTwiceClause->literals;

   EqnSetProp(sameVarTwice->literal, EPIsOriented);
   sameVarTwiceClause->date = 1;


   sameVarTwice->side    = LeftSide;
   sameVarTwice->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, sameVarTwice);

   fprintf(stderr, "\nCurrently in the PDT: \n");
   PDTreePrint(stderr, proofstate->processed_pos_rules->demod_index);

   fprintf(stderr, "\nFifth to match :");
   TermPrint(stderr, sameVarTwiceClause->literals->rterm, proofstate->terms->sig, DEREF_ALWAYS);

   PDTreeSearchInit(proofstate->processed_pos_rules->demod_index, sameVarTwiceClause->literals->rterm, SysDateInvalidTime(), true);

   mi = PDTreeFindNextDemodulator(proofstate->processed_pos_rules->demod_index, subst);

   matcher =  GetMatcher(mi);
   if (!matcher)
   {
      fprintf(stderr, "\nNothing found (%p)!!!\n", mi);
   }
   else
   {
      fprintf(stderr, "\nMatcher :");
      TermPrint(stderr, matcher, proofstate->terms->sig, DEREF_ALWAYS);
      fprintf(stderr, "\nWhat is matched : ");
      TermPrint(stderr, MatchInfoMatchedPrefix(mi, sameVarTwiceClause->literals->rterm), proofstate->terms->sig, DEREF_ALWAYS);
   }
}

Term_p rewrite_with_clause_set(OCB_p ocb, TB_p bank, Term_p term,
                  SysDate date, ClauseSet_p
                  demodulators, bool prefer_general,
                                      bool restricted_rw);
void test_rewriting(ProofState_p proofstate, ProofControl_p proofctrl)
{
  /* Parsed file lfho.rewrite.p */
  //1 tcf(i_0_14, plain, ![X4:tA > tA]:$@_var(X4,$@_var(X4,b))=h(X4)).
  //2 tcf(i_0_10, plain, ![X1:tA]:f(X1,f(a,b))=X1).
  //3 tcf(i_0_13, plain, a_predicate(g(b,a))).
  //4 tcf(i_0_12, plain, ![X3:tA > tA]:$@_var(X3,a)=f($@_var(X3,a),$@_var(X3,a))).
  //5 tcf(i_0_15, plain, ![X5:tA > tA > tA]:$@_var(X5,a)=j(b,b,a)).
  //6 tcf(i_0_16, plain, a_predicate(j(b,a,a,b))).
  //7 tcf(i_0_11, plain, ![X2:tA > tA]:a_predicate(f($@_var(X2,c),f(a,b)))).
  //8 tcf(i_0_17, negated_conjecture, ~a_predicate(f(a,f(a,b)))).

   /* Test case 1:
         test rewrite rule f X     (f a b) = X
         after matching    f (Y c) (f a b)
         the rewritten term has to be     -> Y c 
      PASSED!*/
   ClausePos_p tc1 = ClausePosCellAlloc();
   Clause_p    tc1Clause = proofstate->axioms->anchor->succ->succ;
   tc1->clause  = tc1Clause;
   tc1->literal = tc1Clause->literals;

   EqnSetProp(tc1->literal, EPIsOriented);
   tc1Clause->date = 1;

   tc1->side    = LeftSide;
   tc1->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, tc1);

   Term_p firstToTest = proofstate->axioms->anchor->succ->succ->succ->succ
                                                ->succ->succ->succ->literals
                                                ->lterm->args[0];

   /* Test case 2:
         test rewrite rule X   a = f (X   a) (X   a)
         after matching    g c a 
         the rewritten term has to be  -> f (g c a) (g c a) 
      PASSED!*/
   ClausePos_p tc2 = ClausePosCellAlloc();
   Clause_p    tc2Clause = proofstate->axioms->anchor->succ->succ->succ->succ;
   tc2->clause  = tc2Clause;
   tc2->literal = tc2Clause->literals;

   EqnSetProp(tc2->literal, EPIsOriented);
   tc2Clause->date = 1;

   tc2->side    = LeftSide;
   tc2->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, tc2);

   Term_p secondToTest = proofstate->axioms->anchor->succ->succ->succ
                                                 ->literals->lterm->args[0];

   /* Test case 3: Testing of same prefix match
         test rewrite rule X   (X   b) = h X
         after matching    f a (f a b)
         the rewritten term has to be  -> h (f a) 
      PASSED!*/   
   ClausePos_p tc3 = ClausePosCellAlloc();
   Clause_p    tc3Clause = proofstate->axioms->anchor->succ;
   tc3->clause  = tc3Clause;
   tc3->literal = tc3Clause->literals;

   EqnSetProp(tc3->literal, EPIsOriented);
   tc3Clause->date = 1;

   tc3->side    = LeftSide;
   tc3->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, tc3);

   Term_p thirdToTest = proofstate->axioms->anchor->succ->succ->succ->succ
                                                  ->succ->succ->succ->succ
                                                  ->literals->lterm->args[0];

   /* Test case 4: Testing of same prefix match
         test rewrite rule Y     a = j b b a
         after matching    j b a a b (partially)
         the rewritten term has to be  -> j b b a b 
      PASSED!*/   
   ClausePos_p tc4 = ClausePosCellAlloc();
   Clause_p    tc4Clause = proofstate->axioms->anchor->succ->succ->succ->succ->succ;
   tc4->clause  = tc4Clause;
   tc4->literal = tc4Clause->literals;

   EqnSetProp(tc4->literal, EPIsOriented);
   tc4Clause->date = 1;

   tc4->side    = LeftSide;
   tc4->pos     = NULL;
   PDTreeInsert(proofstate->processed_pos_rules->demod_index, tc4);

   Term_p fourthToTest = proofstate->axioms->anchor->succ->succ->succ
                                                   ->succ->succ->succ
                                                   ->literals->lterm->args[0];

   fprintf(stderr, "Currently in the PDT: \n");
   PDTreePrint(stderr, proofstate->processed_pos_rules->demod_index);

   Term_p rewrittenFirst = 
      rewrite_with_clause_set(proofctrl->ocb, proofstate->terms, firstToTest,
                              SysDateInvalidTime(), proofstate->processed_pos_rules, 
                              true, false);

   TermPrint(stderr, firstToTest, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, " - > ");
   TermPrint(stderr, rewrittenFirst, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, ".\n");

   Term_p rewrittenSecond = 
      rewrite_with_clause_set(proofctrl->ocb, proofstate->terms, secondToTest,
                              SysDateInvalidTime(), proofstate->processed_pos_rules, 
                              true, false);

   TermPrint(stderr, secondToTest, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, " - > ");
   TermPrint(stderr, rewrittenSecond, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, ".\n");

   Term_p rewrittenThird = 
      rewrite_with_clause_set(proofctrl->ocb, proofstate->terms, thirdToTest,
                              SysDateInvalidTime(), proofstate->processed_pos_rules, 
                              true, false);

   TermPrint(stderr, thirdToTest, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, " - > ");
   TermPrint(stderr, rewrittenThird, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, ".\n");

   Term_p rewrittenFourth = 
      rewrite_with_clause_set(proofctrl->ocb, proofstate->terms, fourthToTest,
                              SysDateInvalidTime(), proofstate->processed_pos_rules, 
                              true, false);

   TermPrint(stderr, fourthToTest, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, " - > ");
   TermPrint(stderr, rewrittenFourth, proofstate->terms->sig, DEREF_NEVER);
   fprintf(stderr, ".\n");
}

/* 1 - based indexing */
Clause_p get_clause_by_nr(ClauseSet_p set, int idx)
{
  Clause_p res = set->anchor;
  for(int i=0; i<idx; i++)
  {
    res = res->succ;
  }

  //return ClauseFlatCopy(res);
  return res;
}

void print_subst(Subst_p subst, Sig_p sig)
{
  SubstPrint(stderr, subst, sig, DEREF_NEVER);
}

void perform_matching_test(Term_p matcher, Term_p to_match, int exp_res, Sig_p sig)
{
  static int tc_count = 0;
  tc_count++;

  fprintf(stderr, "\n# Matching test case %d\n# term ", tc_count);

  TermPrint(stderr, matcher, sig, DEREF_NEVER);
  fprintf(stderr, " against ");
  TermPrint(stderr, to_match, sig, DEREF_NEVER);
  fprintf(stderr, " .\n");

  Subst_p subst = SubstAlloc();

  int res = SubstComputeMatchHO(matcher, to_match, subst, sig);
  if (res != exp_res)
  {
    fprintf(stderr, "# exptected result was %d, but got result %d.\n", exp_res, res);
  }
  else 
  {
    fprintf(stderr, "# test case passed.\n");
  }

  print_subst(subst, sig);

  SubstDelete(subst);
}


void test_matching(ProofState_p proofstate)
{
  /*
  Clauses: 
  1 -tcf(i_0_13, plain, ![X4:t > t > t]:$@_var(X4,a)=h).
  2 -tcf(i_0_21, plain, ![X11:t]:pred2(g(X11))).
  3 -tcf(i_0_11, plain, ![X2:t, X1:t]:pred(f(X1,b,X2))).
  4 -tcf(i_0_18, plain, ![X9:t, X10:t, X8:t > t > t, X7:t > t]:$@_var(X7,$@_var(X8,b,b))=$@_var(X7,f(X9,X10,X9))).
  5 -tcf(i_0_20, plain, f(a,b,a)=g(a,b)).
  6 -tcf(i_0_15, plain, f(c,a,b)=f(a,a,a)).
  7 -tcf(i_0_10, plain, pred(f(g(a,b),b,h(c)))).
  8 -tcf(i_0_17, plain, f(a,b,f(a,b,b))=f(a,b,f(a,b,a))).
  9 -tcf(i_0_22, negated_conjecture, ~pred2(h)).
  10-tcf(i_0_16, plain, ![X6:t > t]:~pred($@_var(X6,$@_var(X6,b)))).
  11-tcf(i_0_12, plain, ![X3:t > t > t]:~pred($@_var(X3,a,b))).
  12-tcf(i_0_14, plain, ![X5:t > t > t > t]:~pred($@_var(X5,a,a,b))).
  13-tcf(i_0_19, plain, ~pred(f(a,b,f(a,b,f(a,b,b))))).
  */

  /* TC1 f X b Y has to match f (g a b) b (h c) */
  Sig_p sig = proofstate->terms->sig;

  Clause_p clause3 = get_clause_by_nr(proofstate->axioms, 3);
  Term_p fxby = clause3->literals->lterm->args[0];
  Clause_p clause7 = get_clause_by_nr(proofstate->axioms, 7);
  Term_p fgabbhc = clause7->literals->lterm->args[0];


  perform_matching_test(fxby, fgabbhc, 0, sig);
  
  Clause_p clause11 = get_clause_by_nr(proofstate->axioms, 11);
  Term_p xab = clause11->literals->lterm->args[0];
  Clause_p clause6 = get_clause_by_nr(proofstate->axioms, 6);
  Term_p fcab = clause6->literals->lterm;

  perform_matching_test(xab, fcab, 0, sig);

  Clause_p clause1 = get_clause_by_nr(proofstate->axioms, 1);
  Term_p xa = clause1->literals->lterm;
  Term_p faaa = clause6->literals->rterm;

  perform_matching_test(xa, faaa, 1, sig);

  Clause_p clause12 = get_clause_by_nr(proofstate->axioms, 12);
  Term_p yaab = clause12->literals->lterm->args[0];

  perform_matching_test(xab, yaab, 0, sig);

  Clause_p clause10 = get_clause_by_nr(proofstate->axioms, 10);
  Term_p xxb = clause10->literals->lterm->args[0];
  Clause_p clause8  = get_clause_by_nr(proofstate->axioms, 8);
  Term_p fabfabb = clause8->literals->lterm;

  perform_matching_test(xxb, fabfabb, 0, sig);

  Clause_p clause4 = get_clause_by_nr(proofstate->axioms, 4);
  Term_p xybb = clause4->literals->lterm;

  perform_matching_test(xybb, fabfabb, 0, sig);

  Term_p xfzyz = clause4->literals->rterm;
  Term_p fabfaba = clause8->literals->rterm;

  perform_matching_test(xfzyz, fabfaba, 0, sig);

  Clause_p clause13 = get_clause_by_nr(proofstate->axioms, 13);
  Term_p fabfabfabb = clause13->literals->lterm->args[0];

  perform_matching_test(xfzyz, fabfabfabb, -1, sig);

  Clause_p clause5 = get_clause_by_nr(proofstate->axioms, 5);

  perform_matching_test(clause5->literals->lterm, clause5->literals->rterm, 
                        -1, sig);
} 

void set_term_dates(ClauseSet_p set)
{
   Clause_p anchor = set->anchor;
   Clause_p cl = anchor->succ;

   while(cl != anchor)
   {
      cl->date = 10;
      cl = cl->succ;
   }
}

void perform_sr_test(Clause_p clause, int exp_weak, int exp_strong, bool pos, ClauseSet_p set)
{
   int exp = StrongUnitForwardSubsumption ? exp_strong : exp_weak;

   fprintf(stderr, "# clause before SR ");
   ClausePrint(stderr, clause, true);

   if (pos)
   {
      ClausePositiveSimplifyReflect(set, clause);
   }
   else
   {
      ClauseNegativeSimplifyReflect(set, clause);  
   }

   fprintf(stderr, ", clause after SR(%s) ", pos ? "+" : "-" );
   ClausePrint(stderr, clause, true);
   fprintf(stderr, "\n");

   if (exp == ClauseLiteralNumber(clause))
   {
      fprintf(stderr, "+ test passed\n");
   }
   else
   {
      fprintf(stderr, "- test failed (expected %d, got %d)\n", exp, ClauseLiteralNumber(clause));
   }
}

void test_sr(ProofState_p proofstate)
{
   set_term_dates(proofstate->axioms);
   StrongUnitForwardSubsumption = false/* true*/;
/*
   cnf(i_0_1, plain, ($true)).
   cnf(i_0_2, plain, ($true)).
   cnf(i_0_3, plain, ($true)).
   cnf(i_0_4, plain, ($true)).
   cnf(i_0_5, plain, ($true)).
   cnf(i_0_6, plain, ($true)).
   cnf(i_0_7, plain, ($true)).
   cnf(i_0_8, plain, ($true)).
   cnf(i_0_9, plain, ($true)).
   cnf(i_0_10, plain, ($true)).
   tcf(i_0_11, plain, f(a,b)=g(c,d)).
   tcf(i_0_12, plain, ![X1:t > t > t]:$@_var(X1,c)=h).
   tcf(i_0_13, plain, ![X2:t]:f(X2,X2)=g(X2,b)).
   tcf(i_0_14, plain, c=d).
   tcf(i_0_15, plain, ![X3:t]:i(X3)=j(X3)).
   tcf(i_0_16, plain, ![X5:t, X4:t]:i(X4,X5)=j(X5,X4)).
   tcf(i_0_17, plain, (f(a,b,c)!=g(c,d,c)|f(a,b,d)!=g(c,d,c)|f(a,a,c)!=h(b))).
   tcf(i_0_18, plain, ![X6:t]:(f(X6,X6,a)!=g(X6,b,a)|f(a,a,c)!=g(c,b,c))).
   tcf(i_0_19, plain, (g(a,c,d)!=h(d)|g(a,c,d)!=h(c)|g(a,a,a)!=h(b))).
   tcf(i_0_20, plain, ![X7:t > t > t > t > t, X8:t > t > t, X9:t > t > t]:($@_var(X7,a,b,c,d)!=h(d)|$@_var(X8,c)!=h|$@_var(X9,c,a)!=h(a))).
   tcf(i_0_21, plain, (i(a,b)!=j(b,a)|i(a)!=j(a))).
   tcf(i_0_22, plain, ![X11:t, X10:t]:f(X10)!=g(X11)).
   tcf(i_0_23, plain, ![X12:t]:h(X12)!=g(X12,a,b)).
   tcf(i_0_24, plain, (f(a)=g(b)|f(a,b,c)=g(b,b,c))).
   tcf(i_0_25, plain, (h(c)=g(c,a,b)|h(c)=g(d,a,b))).

*/  
   // inserted f(a,b) = g(c,d) 
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 11)));
   // inserted X c = h
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 12)));

   // inserted f X X  = g X b
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 13)));

   // c = d
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 14)));

   // i X = j X
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 15)));

   // i X Y = j Y X
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 16)));

   PDTreePrint(stderr, proofstate->processed_pos_eqns->demod_index);

   perform_sr_test(get_clause_by_nr(proofstate->axioms, 17), 2, 1, true, proofstate->processed_pos_eqns);
   perform_sr_test(get_clause_by_nr(proofstate->axioms, 18), 1, 1, true, proofstate->processed_pos_eqns);
   perform_sr_test(get_clause_by_nr(proofstate->axioms, 19), 2, 1, true, proofstate->processed_pos_eqns);
   perform_sr_test(get_clause_by_nr(proofstate->axioms, 20), 0, 0, true, proofstate->processed_pos_eqns);
   perform_sr_test(get_clause_by_nr(proofstate->axioms, 21), 0, 0, true, proofstate->processed_pos_eqns);

   // inserted f X != g Y
   ClauseSetPDTIndexedInsert(proofstate->processed_neg_units, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 22)));

   // inserted h X != g X a ab
   ClauseSetPDTIndexedInsert(proofstate->processed_neg_units, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 23)));
   PDTreePrint(stderr, proofstate->processed_neg_units->demod_index);

   perform_sr_test(get_clause_by_nr(proofstate->axioms, 24), 1, 1, false, proofstate->processed_neg_units);
   perform_sr_test(get_clause_by_nr(proofstate->axioms, 25), 1, 1, false, proofstate->processed_neg_units);
}

void perform_us_test(ClauseSet_p set, Clause_p clause, bool exp)
{
   Clause_p subsumer = UnitClauseSetSubsumesClause(set, clause);
   if (EQUIV((bool)subsumer, exp))
   {
      fprintf(stderr, "+ test passed");
      if (subsumer)
      {
         fprintf(stderr, " with subsumer ");
         ClausePrint(stderr, subsumer, true);   
      }
      
      fprintf(stderr, " . \n");
   }
   else
   {
      fprintf(stderr, "- test failed\n");
   }
}

void test_us(ProofState_p proofstate)
{
   set_term_dates(proofstate->axioms);

   /*
      cnf(i_0_1, plain, ($true)).
      cnf(i_0_2, plain, ($true)).
      cnf(i_0_3, plain, ($true)).
      cnf(i_0_4, plain, ($true)).
      cnf(i_0_5, plain, ($true)).
      cnf(i_0_6, plain, ($true)).
      cnf(i_0_7, plain, ($true)).
      tcf(i_0_8, plain, ![X1:t]:f(X1)=g(X1)).
      tcf(i_0_9, plain, ![X3:t, X2:t]:f(X2,X3)=h(X3,X2)).
      tcf(i_0_10, plain, ![X4:t > t]:$@_var(X4,$@_var(X4,b))=g($@_var(X4,a),$@_var(X4,b))).
      tcf(i_0_11, plain, (f(a,f(a,b))=g(f(a,a),f(a,b))|a=b)).
      tcf(i_0_12, plain, f(a,b)=g(a,c)).
      tcf(i_0_13, plain, f(a,b)=g(a,b)).
      tcf(i_0_14, plain, (f(a,b)=g(b,a)|a=b)).
      tcf(i_0_15, plain, ![X5:t > t]:$@_var(X5,a)!=f(b,c)).
      tcf(i_0_16, plain, ![X7:t, X6:t]:g(X6,X7)!=h(X7,X6)).
      tcf(i_0_17, plain, f(a,a)!=f(b,c)).
      tcf(i_0_18, plain, f(a,a)!=f(b,b)).
      tcf(i_0_19, plain, (g(a,a)!=h(a,b)|g(b,b)!=h(a,a))).
      tcf(i_0_20, plain, (h(a,a)!=g(b,b)|g(a,b)!=h(b,a))).
      tcf(i_0_21, plain, ![X9:t, X8:t]:h(X8,X9)!=g(X9,X8)).
   */

   // f X  =  g X
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 8)));

   // f X Y  =  g Y X
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 9)));

   // X (X b) = g (X a) (X b)
   ClauseSetPDTIndexedInsert(proofstate->processed_pos_eqns, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 10)));

   //subsumed by ep3
   perform_us_test(proofstate->processed_pos_eqns, 
                   get_clause_by_nr(proofstate->axioms, 11), true);
   perform_us_test(proofstate->processed_pos_eqns, 
                   get_clause_by_nr(proofstate->axioms, 12), false);
   //subsumed by ep1 
   perform_us_test(proofstate->processed_pos_eqns, 
                   get_clause_by_nr(proofstate->axioms, 13), true);
   //subsumed by ep2
   perform_us_test(proofstate->processed_pos_eqns, 
                   get_clause_by_nr(proofstate->axioms, 14), true);

   // Y a != f b c
   ClauseSetPDTIndexedInsert(proofstate->processed_neg_units, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 15)));

   // g X Y != h Y X
   ClauseSetPDTIndexedInsert(proofstate->processed_neg_units, 
                             ClauseFlatCopy(get_clause_by_nr(proofstate->axioms, 16)));

   //subsumed by en1
   perform_us_test(proofstate->processed_neg_units, 
                   get_clause_by_nr(proofstate->axioms, 17), true);
   perform_us_test(proofstate->processed_neg_units, 
                   get_clause_by_nr(proofstate->axioms, 18), false);
   perform_us_test(proofstate->processed_neg_units, 
                   get_clause_by_nr(proofstate->axioms, 19), false);
   //subsumed by en2
   perform_us_test(proofstate->processed_neg_units, 
                   get_clause_by_nr(proofstate->axioms, 20), true);
   //subsumed by en2
   perform_us_test(proofstate->processed_neg_units, 
                   get_clause_by_nr(proofstate->axioms, 21), true);

}

void perform_unif_test_once(Term_p termA, Term_p termB, Sig_p sig, bool exp, const char* subst_str)
{
   static int tc_nr = 0;
   tc_nr++;

   Subst_p subst = SubstAlloc();

   fprintf(stderr, "# test %d (", tc_nr);
   TermPrint(stderr, termA, sig, DEREF_NEVER);
   fprintf(stderr, " === ");
   TermPrint(stderr, termB, sig, DEREF_NEVER);
   fprintf(stderr, ")\n");


   UnificationResult ur = SubstComputeMguHO(termA, termB, subst, sig);
   if (EQUIV(UnifFailed(ur), exp))
   {
      fprintf(stderr, "- test %d failed (with expected substitution %s, got ", tc_nr, subst_str);
      SubstPrint(stderr, subst, sig, DEREF_NEVER);
      fprintf(stderr, ").\n");

   }
   else
   {
      fprintf(stderr, "+ test %d succeded (with expected substitution %s, got ", tc_nr, subst_str);
      SubstPrint(stderr, subst, sig, DEREF_NEVER);
      fprintf(stderr, ").\n");

      fprintf(stderr, "? after matching: ");
      TermPrint(stderr, termA, sig, DEREF_ALWAYS);
      fprintf(stderr, " === ");
      TermPrint(stderr, termB, sig, DEREF_ALWAYS);
      fprintf(stderr, ".\n");
   }

   fprintf(stderr, "? (side %s - remaining %d )\n", GetSideStr(ur), ur.term_remaining);
   SubstDelete(subst);
}

void perform_unif_test(Term_p termA, Term_p termB, Sig_p sig, bool exp, const char* subst_str)
{
   perform_unif_test_once(termA, termB, sig, exp, subst_str);
   perform_unif_test_once(termB, termA, sig, exp, subst_str);
}

Term_p get_term_from_clause(ClauseSet_p set, int idx)
{
   Clause_p cl = get_clause_by_nr(set, idx);
   return cl->literals->lterm->args[0];
}

void test_unification(ProofState_p p)
{
   /* 
   1   tcf(i_0_23, plain, p(g(a))).
   2   tcf(i_0_30, plain, ![X20:t, X19:t > t > t]:p($@_var(X19,c,X20))).
   3   tcf(i_0_24, plain, ![X10:t > t > t]:p($@_var(X10,a,b))).
   4   tcf(i_0_13, plain, ![X4:t]:p(f(X4,a,X4))).
   5   tcf(i_0_11, plain, ![X2:t, X1:t]:p(f(X1,a,X2))).
   6   tcf(i_0_29, plain, ![X18:t, X17:t > t > t > t]:p($@_var(X17,a,X18,b))).
   7   tcf(i_0_31, plain, ![X22:t, X21:t]:p(f(X21,i(X21),X22))).
   8   tcf(i_0_12, plain, ![X3:t]:p(f(b,a,X3))).
   9   tcf(i_0_32, plain, ![X24:t, X23:t]:p(f(g(X23),X24,X23))).
   10  tcf(i_0_26, plain, ![X11:t > t > t > t]:p($@_var(X11,a,a,b))).
   11  tcf(i_0_14, plain, p(f(a,a,b))).
   12  tcf(i_0_25, plain, p(f(a,a,b))).
   13  tcf(i_0_22, plain, p(f(a,b,c))).
   14  tcf(i_0_27, plain, ![X13:t > t, X12:t > t > t]:p($@_var(X12,$@_var(X13,c),$@_var(X12,a,b)))).
   15  tcf(i_0_15, plain, ![X7:t, X6:t, X5:t]:p(f(f(X5,X6,X7),X5,X6))).
   16  tcf(i_0_18, plain, p2(f(g(b),g(c)))).
   17  tcf(i_0_20, plain, ![X8:t > t]:p(f($@_var(X8,a),$@_var(X8,b),c))).
   18  tcf(i_0_19, plain, p(f(g(b),g(c),g(a)))).
   19  tcf(i_0_16, plain, p(f(f(a,b,c),a,b))).
   20  tcf(i_0_17, plain, p(f(f(a,b,c),b,b))).
   21  tcf(i_0_28, plain, ![X15:t, X16:t, X14:t > t]:p(h(a,b,$@_var(X14,c),h(a,b,X15,X16)))).
   22  tcf(i_0_21, plain, ![X9:t]:p(f(f(a,b,a),f(a,b,b),X9))).


   */

   Term_p fxay = get_term_from_clause(p->axioms, 5);
   Term_p fbaz = get_term_from_clause(p->axioms, 8);

   perform_unif_test(fxay, fbaz, p->signature, true, "{X <- b, Y <- Z (or Z <- Y)}");

   Term_p fxax = get_term_from_clause(p->axioms, 4);
   Term_p faab = get_term_from_clause(p->axioms, 11);

   perform_unif_test(fxax, faab, p->signature, false, "{ }");

   Term_p ffxyzxy = get_term_from_clause(p->axioms, 15);
   Term_p ffabcab = get_term_from_clause(p->axioms, 19);

   perform_unif_test(ffxyzxy, ffabcab, p->signature, true, "{X <- a, Y <- b, Z <- c }");

   Term_p ffabcbb = get_term_from_clause(p->axioms, 20);

   perform_unif_test(ffxyzxy, ffabcbb, p->signature, false, "{ }");

   Term_p fgbgc = get_term_from_clause(p->axioms, 16);
   Term_p fgbgcga = get_term_from_clause(p->axioms, 18);

   perform_unif_test(fgbgc, fgbgcga, p->signature, true, "{ }");

   Term_p fxaxbc = get_term_from_clause(p->axioms, 17);
   Term_p ffabafabby = get_term_from_clause(p->axioms, 22);

   perform_unif_test(fxaxbc, ffabafabby, p->signature, true, "{X <- f(a,b), Y <- c}");

   Term_p fabc = get_term_from_clause(p->axioms, 13);
   Term_p ga = get_term_from_clause(p->axioms, 1);

   perform_unif_test(fabc, ga, p->signature, false, "{ }");

   Term_p xab = get_term_from_clause(p->axioms, 3);

   perform_unif_test(xab, faab, p->signature, true, "{ X <- f(a) }");

   Term_p yaab = get_term_from_clause(p->axioms, 10);

   perform_unif_test(xab, yaab, p->signature, true, "{ X <- Y a }");

   Term_p xycxab = get_term_from_clause(p->axioms, 14);
   Term_p habzchabwu = get_term_from_clause(p->axioms, 21);

   perform_unif_test(xycxab, habzchabwu, p->signature, true, "{ X <- f a b, Y <- Z, W <- a, U <- b }");

   Term_p xayb = get_term_from_clause(p->axioms, 6);
   Term_p gcw = get_term_from_clause(p->axioms, 2);

   perform_unif_test(xayb, gcw, p->signature, true, "{ Z <- X a , Y <- c, W <- b }");

   //test variants
   Term_p xayb_copy = TermCopyKeepVars(xayb, DEREF_NEVER);
   
   Subst_p var_copy = SubstAlloc();
   SubstNormTerm(xayb_copy, var_copy, p->freshvars);
   xayb_copy = TBInsert(p->terms, xayb_copy, DEREF_ALWAYS);
   SubstDelete(var_copy);

   perform_unif_test(xayb, xayb_copy, p->signature, true, "{X <- Y, Z <- W or permutation}");

   Term_p fxhxy = get_term_from_clause(p->axioms, 7);
   Term_p fgzwz = get_term_from_clause(p->axioms, 9);

   perform_unif_test(fxhxy, fgzwz, p->signature, true, "{X <- g Z, W <- h (g Z), Y <- Z}");   

}

Clause_p clause_set_subsumes_clause(ClauseSet_p set, Clause_p sub_candidate);

void perform_subsumption_test(ClauseSet_p set, Clause_p test, Clause_p exp_subsumer, Sig_p sig)
{
   static int test_nr = 0;
   test_nr++;

   test = ClauseFlatCopy(ClauseSubsumeOrderSortLits(test));
   test->weight = ClauseStandardWeight(test);

   fprintf(stderr, "# test %d :  ", test_nr );
   ClausePrint(stderr, test, true);
   fprintf(stderr, ".\n");

   FVPackedClause_p test_fvp =  FVIndexPackClause(test, set->fvindex);
   Clause_p subsumer = ClauseSetSubsumesFVPackedClause(set, test_fvp);

   //Clause_p subsumer = clause_set_subsumes_clause(set, test);

   bool passed = subsumer == exp_subsumer;
   fprintf(stderr, "%c test %s with subsumer ", passed ? '+' : '-', passed ?  "passed" : "failed" );
   if (subsumer)
   {
      ClausePrint(stderr, subsumer, true);
      fprintf(stderr, ".\n");
   }
   else 
   {
      fprintf(stderr, " (none).\n");
   }

   if (!passed)
   {
      fprintf(stderr, "- expected ");
      if (exp_subsumer)
      {
         ClausePrint(stderr, exp_subsumer, true);
         fprintf(stderr, ".\n");
      }
      else 
      {
         fprintf(stderr, " (none).\n");
      }      
   }
}

void test_subsumption(ProofState_p p)
{
   /*
   1   tcf(i_0_11, plain, ![X1:t > t, X2:t > t, X3:t]:($@_var(X1,a)=$@_var(X2,b)|g(b,X3)=f(a,X3))).
   2   tcf(i_0_14, plain, ![X2:t, X1:t]:(h(X1)=f(h(X1),h(X1))|f(X1,X2)=g(f(X1,X2),h(X2)))).
   3   tcf(i_0_17, plain, ![X1:t > t, X3:t, X2:t]:($@_var(X1,$@_var(X1,a))=g(b,$@_var(X1,b))|f(i(X2),i(X3))=h(g(X2,X3)))).
   4   tcf(i_0_15, plain, (h(a)=f(h(a),h(a))|f(a,b)=g(f(a,b),h(b))|p(a))).
   5   tcf(i_0_16, plain, (h(c)=f(h(c),h(c))|f(a,b)=g(f(a,b),h(b))|p(a))).
   6   tcf(i_0_13, plain, ![X1:t > t > t]:(g(b)=f(a)|g(b,c)=f(a,c)|$@_var(X1,$@_var(X1,a,b),$@_var(X1,b,c))=$@_var(X1,$@_var(X1,a,a),$@_var(X1,a,a)))).
   7   tcf(i_0_20, plain, (f(i(c),i(c))=h(g(a,a))|g(b,f(a,b))=f(a,f(a,a))|p(a))).
   8   tcf(i_0_18, plain, (f(i(c),i(c))=h(g(c,c))|g(b,f(a,b))=f(a,f(a,a))|p(a))).
   9   tcf(i_0_19, plain, (f(i(c),i(c))=h(g(c,c))|g(b,f(b,b))=f(a,f(a,a))|p(a))).
   10   tcf(i_0_12, plain, ![X1:t > t > t, X2:t > t > t > t]:(i(b)=h(a)|$@_var(X1,b,b)=$@_var(X1,a,b)|g(b,b)=f(a,b)|$@_var(X2,b,c,c)=$@_var(X2,a,b,c)|p(a))).

   */

   ClauseSet_p set = p->processed_non_units;

   //ClauseSetPrint(stderr, p->axioms, true);
   Clause_p a1 = ClauseSetExtractEntry(get_clause_by_nr(p->axioms, 1));
   //fprintf(stderr, "- \n");
   //ClauseSetPrint(stderr, p->axioms, true);
   Clause_p a2 = ClauseSetExtractEntry(get_clause_by_nr(p->axioms, 1));
   //fprintf(stderr, "- \n");
   //ClauseSetPrint(stderr, p->axioms, true);
   Clause_p a3 = ClauseSetExtractEntry(get_clause_by_nr(p->axioms, 1));
   //fprintf(stderr, "- \n");
   //ClauseSetPrint(stderr, p->axioms, true);

   /*tcf(i_0_13, plain, (g(b)=f(a)|g(b,c)=f(a,c))).
   tcf(i_0_12, plain, (i(b)=h(a)|g(b,b)=f(a,b)|p(a))).
   tcf(i_0_15, plain, (h(a)=f(h(a),h(a))|f(a,b)=g(f(a,b),h(b))|p(a))).
   tcf(i_0_16, plain, (h(c)=f(h(c),h(c))|f(a,b)=g(f(a,b),h(b))|p(a))).
   tcf(i_0_20, plain, (f(i(c),i(c))=h(g(a,a))|g(b,f(a,b))=f(a,f(a,a))|p(a))).
   tcf(i_0_18, plain, (f(i(c),i(c))=h(g(c,c))|g(b,f(a,b))=f(a,f(a,a))|p(a))).
   tcf(i_0_19, plain, (f(i(c),i(c))=h(g(c,c))|g(b,f(b,b))=f(a,f(a,a))|p(a))).*/


   ClauseSetIndexedInsert(set, FVIndexPackClause(a1, set->fvindex));
   ClauseSetIndexedInsert(set, FVIndexPackClause(a2, set->fvindex));
   ClauseSetIndexedInsert(set, FVIndexPackClause(a3, set->fvindex));

   FVIndexPrint(stderr, set->fvindex->index, true);

   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 7), a1, p->signature);
   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 3), NULL, p->signature);
   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 1), a2, p->signature);
   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 2), NULL, p->signature);
   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 5), a3, p->signature);
   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 6), NULL, p->signature);
   perform_subsumption_test(set, get_clause_by_nr(p->axioms, 4), NULL, p->signature);
}

CompareResult invert(CompareResult x)
{
   switch(x)
   {
      case to_greater:
         x = to_lesser;
         break;
      case to_lesser:
         x = to_greater;
         break;
   }
   return x;
}

extern char* POCompareSymbol[];
void perform_one_kbo_test(OCB_p ocb, Term_p left, Term_p right, CompareResult exp)
{
   fprintf(stderr, "? comparing ");
   TermPrint(stderr, left, ocb->sig, DEREF_NEVER);
   fprintf(stderr, " and ");
   TermPrint(stderr, right, ocb->sig, DEREF_NEVER);
   fprintf(stderr, "\n");

   CompareResult res =  KBO6Compare(ocb, left, right, DEREF_NEVER, DEREF_NEVER);
   if (res == exp)
   {
      fprintf(stderr, "+ test case passed with result %s\n", POCompareSymbol[res]);
   }
   else
   {
      fprintf(stderr, "- test case failed - exp = %s, res = %s\n",
                      POCompareSymbol[exp], POCompareSymbol[res]);
   }
}

void perform_kbo_test(OCB_p ocb, Term_p left, Term_p right, CompareResult exp)
{
   static int tc_nr = 0;

   fprintf(stderr, "# KBO test case %d\n", ++tc_nr);
   perform_one_kbo_test(ocb, left, right, exp);

   fprintf(stderr, "# KBO test case %d\n", ++tc_nr);
   perform_one_kbo_test(ocb, right, left, invert(exp));
}

void test_kbo(ProofState_p p, ProofControl_p c)
{
   /*
   1   tcf(i_0_29, plain, ![X11:t, X12:t, X10:t]:f2(X10,X11)=f2(X12,X10)).
   2   tcf(i_0_25, plain, ![X3:t > t > t, X2:t > t > t]:$@_var(X2,a,b)=$@_var(X3,a,b)).
   3   tcf(i_0_24, plain, ![X1:t > t > t]:$@_var(X1,b,a)=$@_var(X1,a,b)).
   4   tcf(i_0_21, plain, p2(f3(a,b))).
   5   tcf(i_0_22, plain, p2(f3(c,d))).
   6   tcf(i_0_26, plain, ![X4:t > t > t]:$@_var(X4,a,b)=f2(a,b)).
   7   tcf(i_0_20, plain, p(f3(a,b,c))).
   8   tcf(i_0_23, plain, p(f3(i,j,k))).
   9   tcf(i_0_19, plain, g1(f1(f1(f0)))=f2(f1(f0),f0)).
   10  tcf(i_0_27, plain, ![X5:t > t > t]:f3($@_var(X5,a,b),d,c)=f3($@_var(X5,a,b),c,d)).
   11  tcf(i_0_28, plain, ![X7:t, X8:t, X9:t > t > t, X6:t]:h(f3(X6,X7,X8),$@_var(X9,c,d))=h(g2(X6,c),f3(a,b,c))).

   */

   /* TC 1: g1 @ (f1 @ (f1 @ f0)) > f2 @ (f1 @ f0) @ f0 */
   Clause_p tc1_cl   = get_clause_by_nr(p->axioms, 9);
   Term_p   tc1_l    = tc1_cl->literals->lterm;
   Term_p   tc1_r    = tc1_cl->literals->rterm;
   perform_kbo_test(c->ocb, tc1_l, tc1_r, to_greater);

   /* TC2: f3 @ a @ b @ c > f3 @ a @ b*/
   Term_p   tc2_l   = get_term_from_clause(p->axioms, 7);
   Term_p   tc2_r   = get_term_from_clause(p->axioms, 4);
   perform_kbo_test(c->ocb, tc2_l, tc2_r, to_greater);

   /* TC3: f3 @ c @ d < f3 @ i @ j @ k*/
   Term_p   tc3_l   = get_term_from_clause(p->axioms, 5);
   Term_p   tc3_r   = get_term_from_clause(p->axioms, 8);
   perform_kbo_test(c->ocb, tc3_l, tc3_r, to_lesser);

   /* TC4: X @ a @ b > X @ b @ a */
   Clause_p tc4_cl  = get_clause_by_nr(p->axioms, 3);
   Term_p   tc4_l   = tc4_cl->literals->rterm;
   Term_p   tc4_r   = tc4_cl->literals->lterm;
   perform_kbo_test(c->ocb, tc4_l, tc4_r, to_greater);

   /* TC5: X @ a @ b > X @ b @ a */
   Clause_p tc5_cl  = get_clause_by_nr(p->axioms, 2);
   Term_p   tc5_l   = tc5_cl->literals->lterm;
   Term_p   tc5_r   = tc5_cl->literals->rterm;
   perform_kbo_test(c->ocb, tc5_l, tc5_r, to_uncomparable);

   /* TC6: f3 @ (X @ a @ b) @ c @ d > f3 @ (X @ a @ b) @ d @ c */
   Clause_p tc6_cl  = get_clause_by_nr(p->axioms, 10);
   Term_p   tc6_l   = tc6_cl->literals->rterm;
   Term_p   tc6_r   = tc6_cl->literals->lterm;
   perform_kbo_test(c->ocb, tc6_l, tc6_r, to_greater);

   /* TC7: h @ (f3 @ X @ Y @ Z) @ (W @ c @ d) < h @ (g2 @ X @ c) @ (f3 @ a @ b @ c) */
   Clause_p tc7_cl  = get_clause_by_nr(p->axioms, 11);
   Term_p   tc7_l   = tc7_cl->literals->rterm;
   Term_p   tc7_r   = tc7_cl->literals->lterm;
   perform_kbo_test(c->ocb, tc7_l, tc7_r, to_greater);

   /* TC8: f2 @ X @ Z ? f2 @ Y @ X */
   Clause_p tc8_cl  = get_clause_by_nr(p->axioms, 1);
   Term_p   tc8_l   = tc8_cl->literals->lterm;
   Term_p   tc8_r   = tc8_cl->literals->rterm;
   perform_kbo_test(c->ocb, tc8_l, tc8_r, to_uncomparable);
}

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
      //goto cleanup1;
   }

   if(relevancy_pruned || incomplete)
   {
      proofstate->state_is_complete = false;
   }
   if(BuildProofObject)
   {
      FormulaSetArchive(proofstate->f_axioms, proofstate->f_ax_archive);
   }
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
   if (!strstr(state->argv[0], "simplify.reflect") && !strstr(state->argv[0], "unit.subsumption"))
   {
      if(!no_preproc)
      {
        if(BuildProofObject)
        {
           ClauseSetArchive(proofstate->ax_archive, proofstate->axioms);
           if(proofstate->watchlist)
           {
              ClauseSetArchive(proofstate->ax_archive, proofstate->watchlist);
           }
        }
        preproc_removed = ClauseSetPreprocess(proofstate->axioms,
                                              proofstate->watchlist,
                                              proofstate->archive,
                                              proofstate->tmp_terms,
                                              eqdef_incrlimit,
                                              eqdef_maxclauses);

        fprintf(stderr, "preproc removed : %d\n", preproc_removed);
      }


      fprintf(stderr, "Should sort now.\n");
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
      ProofStateInit(proofstate, proofcontrol);
      ProofStateInitWatchlist(proofstate, proofcontrol->ocb,
                              watchlist_filename, parse_format);

      VERBOUT2("Prover state initialized\n");

   }

   fprintf(stderr, "Clauses: \n");
   ClauseSetPrint(stderr, proofstate->axioms, true);
   fprintf(stderr, "Clauses printed. \n");

   //test_pdts(proofstate);
   if (strstr(state->argv[0], "rewrite"))
   {
      for(Clause_p cl = proofstate->axioms->anchor; cl != proofstate->axioms->anchor; cl = cl->succ)
      {
        cl->date = -100; // to make sure it is rewritten.
        EqnListSetProp(cl->literals, EPIsOriented);
      }
      test_rewriting(proofstate, proofcontrol);   
   }
   else if (strstr(state->argv[0], "matching"))
   {
      test_matching(proofstate);
   }
   else if (strstr(state->argv[0], "simplify.reflect"))
   {
      test_sr(proofstate);
   }
   else if (strstr(state->argv[0], "unit.subsumption"))
   {
      test_us(proofstate);
   }
   else if (strstr(state->argv[0], "unification"))
   {
      test_unification(proofstate);
   }
   else if (strstr(state->argv[0], "lfho.subsumption"))
   {
      test_subsumption(proofstate);
   }
   else if (strstr(state->argv[0], "kbo"))
   {
      if (OCBFunCompare(proofcontrol->ocb, SigFindFCode(proofstate->signature, "a"), SigFindFCode(proofstate->signature, "b"))
          != to_greater || OCBFunCompare(proofcontrol->ocb, SigFindFCode(proofstate->signature, "c"), SigFindFCode(proofstate->signature, "d"))
          != to_greater || OCBFunCompare(proofcontrol->ocb, SigFindFCode(proofstate->signature, "g2"), SigFindFCode(proofstate->signature, "f3"))
          != to_greater)
      {
         Error("E IS IGNORING YOUR PRECEDENCE!\n", SYNTAX_ERROR);

      }
      test_kbo(proofstate, proofcontrol);
   }
   else
   {
      test_pdts(proofstate);
   }
   
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
            fprintf(stdout, "E " VERSION " " E_NICKNAME "\n");
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
            BuildProofObject = MAX(CLStateGetIntArgCheckRange(handle, arg, 0, 3),
                                   BuildProofObject);
            print_derivation = POList;
            break;
      case OPT_PROOF_GRAPH:
            BuildProofObject = MAX(1, BuildProofObject);
            print_derivation = CLStateGetIntArg(handle, arg)+1;
            break;
      case OPT_FULL_DERIV:
            print_full_deriv = true;
            break;
      case OPT_FORCE_DERIV:
            force_deriv_output = CLStateGetIntArgCheckRange(handle, arg, 0, 2);
            BuildProofObject = MAX(1, BuildProofObject);
            break;
      case OPT_RECORD_GIVEN_CLAUSES:
            BuildProofObject = MAX(1, BuildProofObject);
            ProofObjectRecordsGCSelection = true;
            break;
      case OPT_TRAINING:
            BuildProofObject = MAX(1, BuildProofObject);
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
            h_parms->ordertype = AUTO;
            sine = "Auto";
            break;
      case OPT_SATAUTO:
            h_parms->heuristic_name = "Auto";
            h_parms->ordertype = AUTO;
            break;
      case OPT_AUTODEV:
            h_parms->heuristic_name = "AutoDev";
            h_parms->ordertype = AUTODEV;
            sine = "Auto";
            break;
      case OPT_SATAUTODEV:
            h_parms->heuristic_name = "AutoDev";
            h_parms->ordertype = AUTODEV;
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
      case OPT_FILTER_LIMIT:
            h_parms->filter_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_FILTER_COPIES_LIMIT:
            h_parms->filter_copies_limit = CLStateGetIntArg(handle, arg);
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
            h_parms->pm_type = ParamodAlwaysSim;
            break;
      case OPT_USE_ORIENTED_SIM_PARAMOD:
            h_parms->pm_type = ParamodOrientedSim;
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
      case OPT_REWEIGHT_LIMIT:
            h_parms->reweight_limit = CLStateGetIntArg(handle, arg);
            break;
      case OPT_ORDERING:
            if(strcmp(arg, "Auto")==0)
            {
               h_parms->ordertype = AUTO;
            }
            else if(strcmp(arg, "AutoCASC")==0)
            {
               h_parms->ordertype = AUTOCASC;
            }
            else if(strcmp(arg, "AutoDev")==0)
            {
               h_parms->ordertype = AUTODEV;
            }
            else if(strcmp(arg, "AutoSched0")==0)
            {
               h_parms->ordertype = AUTOSCHED0;
            }
            else if(strcmp(arg, "AutoSched1")==0)
            {
               h_parms->ordertype = AUTOSCHED1;
            }
            else if(strcmp(arg, "AutoSched2")==0)
            {
               h_parms->ordertype = AUTOSCHED2;
            }
            else if(strcmp(arg, "AutoSched3")==0)
            {
               h_parms->ordertype = AUTOSCHED3;
            }
            else if(strcmp(arg, "AutoSched4")==0)
            {
               h_parms->ordertype = AUTOSCHED4;
            }
            else if(strcmp(arg, "AutoSched5")==0)
            {
               h_parms->ordertype = AUTOSCHED5;
            }
            else if(strcmp(arg, "AutoSched6")==0)
            {
               h_parms->ordertype = AUTOSCHED6;
            }
            else if(strcmp(arg, "AutoSched7")==0)
            {
               h_parms->ordertype = AUTOSCHED7;
            }
            else if(strcmp(arg, "Optimize")==0)
            {
               h_parms->ordertype = OPTIMIZE_AX;
            }
            else if(strcmp(arg, "LPO")==0)
            {
               h_parms->ordertype = LPO;
            }
            else if(strcmp(arg, "LPOCopy")==0)
            {
               h_parms->ordertype = LPOCopy;
            }
            else if(strcmp(arg, "LPO4")==0)
            {
               h_parms->ordertype = LPO4;
            }
            else if(strcmp(arg, "LPO4Copy")==0)
            {
               h_parms->ordertype = LPO4Copy;
            }
            else if(strcmp(arg, "KBO")==0)
            {
               h_parms->ordertype = KBO;
            }
            else if(strcmp(arg, "KBO6")==0)
            {
               h_parms->ordertype = KBO6;
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
            h_parms->to_weight_gen = TOTranslateWeightGenMethod(arg);
            if(!h_parms->to_weight_gen)
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
            h_parms->to_pre_weights = arg;
            break;
      case OPT_TO_PRECGEN:
            h_parms->to_prec_gen = TOTranslatePrecGenMethod(arg);
            if(!h_parms->to_prec_gen)
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
      case OPT_TO_CONSTWEIGHT:
            h_parms->to_const_weight = CLStateGetIntArg(handle, arg);
            if(h_parms->to_const_weight<=0)
            {
               Error("Argument to option -c (--order-constant-weight) "
                     "has to be > 0", USAGE_ERROR);
            }
            break;
      case OPT_TO_PRECEDENCE:
            h_parms->to_pre_prec = arg;
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
            h_parms->no_lit_cmp = true;
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
            RewriteStrongRHSInst = true;
            break;
      case OPT_STRONGSUBSUMPTION:
            StrongUnitForwardSubsumption = true;
            break;
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
                     "'None', 'AC', 'SS', or 'All'.", USAGE_ERROR);
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
