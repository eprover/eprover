/*-----------------------------------------------------------------------

  File  : ccl_proofstate.h

  Author: Stephan Schulz

  Contents

  Proof objects describing the state of a proof attempt (i.e. all
  information relevant to the calculus, but not information describing
  control).

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Oct 14 22:46:13 MET DST 1998 (extracted from
  CONTROL/cco_proofstate.h)

-----------------------------------------------------------------------*/

#ifndef CTO_PROOFSTATE

#define CTO_PROOFSTATE

#include <cio_output.h>
#include <ccl_def_handling.h>
#include <ccl_garbage_coll.h>
#include <ccl_global_indices.h>
#include <ccl_rewrite.h>
#include <picosat.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Proof state */

typedef struct proofstatecell
{
   TypeBank_p    type_bank;
   Sig_p         signature;
   long          original_symbols;
   TB_p          terms;
   TB_p          tmp_terms;
   IntMap_p      choice_opcodes;
   VarBank_p     freshvars;
   FormulaSet_p  f_ax_archive;
   FormulaSet_p  f_axioms;
   ClauseSet_p   axioms;
   ClauseSet_p   ax_archive;
   ClauseSet_p   processed_pos_rules;
   ClauseSet_p   processed_pos_eqns;
   ClauseSet_p   processed_neg_units;
   ClauseSet_p   processed_non_units;
   ClauseSet_p   unprocessed;
   ClauseSet_p   tmp_store;
   ClauseSet_p   eval_store;
   ClauseSet_p   archive;
   FormulaSet_p  f_archive;
   PStack_p      extract_roots;
   GlobalIndices gindices;
   bool          fvi_initialized; /* Are the feature vector
                                     indices set up? */
   FVCollect_p   fvi_cspec;
   ClauseSet_p   demods[3];       /* Synonyms for
                                     processed_pos_rules and
                                     processed_pos_eqns */
   ClauseSet_p   watchlist;
   GlobalIndices wlindices;
   bool          state_is_complete;
   bool          has_interpreted_symbols;
   DefStore_p    definition_store;
   FVCollect_p   def_store_cspec;

   bool          status_reported;
   long          answer_count;

   unsigned long processed_count;
   unsigned long proc_trivial_count;
   unsigned long proc_forward_subsumed_count;
   unsigned long proc_non_trivial_count;
   unsigned long other_redundant_count; /* Intermediate
                                           filtering */
   unsigned long non_redundant_deleted;
   unsigned long backward_subsumed_count;
   unsigned long backward_rewritten_count;
   unsigned long backward_rewritten_lit_count;
   unsigned long rw_count;
   unsigned long generated_count;
   unsigned long aggressive_forward_subsumed_count;
   unsigned long generated_lit_count;
   unsigned long non_trivial_generated_count;
   unsigned long context_sr_count;
   unsigned long paramod_count;
   unsigned long factor_count;
   unsigned long neg_ext_count;
   unsigned long resolv_count;
   unsigned long disequ_deco_count;
   unsigned long satcheck_count;
   unsigned long satcheck_success;
   unsigned long satcheck_satisfiable;
   unsigned long satcheck_full_size; // Number of prop. clauses
   unsigned long satcheck_actual_size; // ...after purity reduction
   unsigned long satcheck_core_size; // ...in unsat core (if any)
   double        satcheck_preproc_time;
   double        satcheck_encoding_time;
   double        satcheck_solver_time;
   /* Times for the final successful run (if any) */
   double        satcheck_preproc_stime;
   double        satcheck_encoding_stime;
   double        satcheck_solver_stime;
   unsigned long filter_orphans_base;  /* Number of back-simplified
                                          clauses at last orphan
                                          filtering */
   unsigned long long forward_contract_base; /* Number of processed
                                                clauses at last
                                                forward-contraction */

   /* The following are only set by ProofStateAnalyse() after
      DerivationCompute() at the end of the proof search. */
   unsigned long gc_count;
   unsigned long gc_used_count;
}ProofStateCell, *ProofState_p;

typedef enum
{
   TSPrintPos = 1,
   TSPrintNeg = 2,
   TSAverageData = 4
}TrainingSelector;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ProofStateCellAlloc() \
   (ProofStateCell*)SizeMalloc(sizeof(ProofStateCell))
#define ProofStateCellFree(junk) \
   SizeFree(junk, sizeof(ProofStateCell))

ProofState_p ProofStateAlloc(FunctionProperties free_symb_prop);
void         ProofStateLoadWatchlist(ProofState_p state,
                                     char* watchlist_filename,
                                     IOFormat parse_format);


void         ProofStateInitWatchlist(ProofState_p state, OCB_p ocb);
void         ProofStateResetClauseSets(ProofState_p state, bool term_gc);
void         ProofStateFree(ProofState_p junk);

long         ProofStateProcessDistinct(ProofState_p state);

//void         ProofStateGCMarkTerms(ProofState_p state);
//long         ProofStateGCSweepTerms(ProofState_p state);

#define      ProofStateStorage(state)                   \
   (ClauseSetStorage((state)->unprocessed)+             \
    ClauseSetStorage((state)->processed_pos_rules)+     \
    ClauseSetStorage((state)->processed_pos_eqns)+      \
    ClauseSetStorage((state)->processed_neg_units)+     \
    ClauseSetStorage((state)->processed_non_units)+     \
    ClauseSetStorage((state)->archive)+                 \
    TBStorage((state)->terms))

#define      ProofStateProcCardinality(state)          \
   (ClauseSetCardinality((state)->processed_pos_rules)+   \
    ClauseSetCardinality((state)->processed_pos_eqns)+    \
    ClauseSetCardinality((state)->processed_neg_units)+   \
    ClauseSetCardinality((state)->processed_non_units))

#define      ProofStateUnprocCardinality(state)  \
    ClauseSetCardinality((state)->unprocessed)

#define      ProofStateCardinality(state)          \
   (ProofStateProcCardinality(state)+              \
    ProofStateUnprocCardinality(state))

bool ProofStateIsUntyped(ProofState_p state);
void ProofStateAnalyseGC(ProofState_p state);
void ProofStatePickTrainingExamples(ProofState_p state,
                                    PStack_p pos_examples,
                                    PStack_p neg_examples);
void ProofStateTrain(ProofState_p state, bool print_pos, bool print_neg);
void ProofStateStatisticsPrint(FILE* out, ProofState_p state);
void ProofStatePrint(FILE* out, ProofState_p state);
void ProofStatePropDocQuote(FILE* out, int level,
                            FormulaProperties prop,
                            ProofState_p state, char* comment);

#define ProofStateAxNo(state) (ClauseSetCardinality((state)->axioms)+\
                               FormulaSetCardinality((state)->f_axioms))

#define WATCHLIST_INLINE_STRING "Use inline watchlist type"
#define WATCHLIST_INLINE_QSTRING "'" WATCHLIST_INLINE_STRING "'"
extern char* UseInlinedWatchList;

void ProofStateResetSATSolver(ProofState_p state);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
