/*-----------------------------------------------------------------------

File  : ccl_proofstate.h

Author: Stephan Schulz

Contents
 
  Proof objects describing the state of a proof attempt (i.e. all
  information relevant to the calculus, but not information describing
  control).

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Sat Jul  5 02:28:25 MET DST 1997
    New
<2> Wed Oct 14 22:46:13 MET DST 1998
    Extracted from CONTROL/cco_proofstate.h

-----------------------------------------------------------------------*/

#ifndef CTO_PROOFSTATE

#define CTO_PROOFSTATE

#include <cio_output.h>
#include <ccl_clausesets.h>
#include <ccl_formula_wrapper.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct proofstatecell
{
   Sig_p             signature;
   long              original_symbols;
   TB_p              original_terms;
   TB_p              terms;
   TB_p              tmp_terms;
   VarBank_p         freshvars;
   FormulaSet_p      f_axioms;
   ClauseSet_p       axioms;
   ClauseSet_p       processed_pos_rules;
   ClauseSet_p       processed_pos_eqns;
   ClauseSet_p       processed_neg_units;
   ClauseSet_p       processed_non_units;
   ClauseSet_p       unprocessed;
   ClauseSet_p       tmp_store;
   bool              fvi_initialized; /* Are the feature vector
                                         indices set up? */
   ClauseSet_p       demods[3];
   ClauseSet_p       watchlist;
   bool              state_is_complete;
   ulong_c           processed_count;
   ulong_c           proc_trivial_count;
   ulong_c           proc_forward_subsumed_count;
   ulong_c           proc_non_trivial_count;
   ulong_c           other_redundant_count;       /* Intermediate
    		            		             filtering */
   ulong_c           non_redundant_deleted;
   ulong_c           backward_subsumed_count;
   ulong_c           backward_rewritten_count;
   ulong_c           backward_rewritten_lit_count;
   ulong_c           generated_count;
   ulong_c           generated_lit_count;
   ulong_c           non_trivial_generated_count;
   ulong_c           context_sr_count;
   ulong_c           paramod_count;
   ulong_c           factor_count;
   ulong_c           resolv_count;   
}ProofStateCell, *ProofState_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define ProofStateCellAlloc() \
   (ProofStateCell*)SizeMalloc(sizeof(ProofStateCell))
#define ProofStateCellFree(junk) \
   SizeFree(junk, sizeof(ProofStateCell))

ProofState_p ProofStateAlloc(void);
void         ProofStateResetClauseSets(ProofState_p state, bool term_gc);
void         ProofStateFree(ProofState_p junk);
void         ProofStateGCMarkTerms(ProofState_p state);
long         ProofStateGCSweepTerms(ProofState_p state);

#define      ProofStateStorage(state) \
   (ClauseSetStorage((state)->unprocessed)+\
    ClauseSetStorage((state)->processed_pos_rules)+\
    ClauseSetStorage((state)->processed_pos_eqns)+\
    ClauseSetStorage((state)->processed_neg_units)+\
    ClauseSetStorage((state)->processed_non_units)+\
    TBStorage((state)->terms))

void ProofStateStatisticsPrint(FILE* out, ProofState_p state);
void ProofStatePrint(FILE* out, ProofState_p state);
void ProofStatePropDocQuote(FILE* out, int level, 
			    ClauseProperties prop,
			    ProofState_p state, char* comment);

#define WATCHLIST_INLINE_STRING "Use inline watchlist type"
#define WATCHLIST_INLINE_QSTRING "'" WATCHLIST_INLINE_STRING "'"
extern char* UseInlinedWatchList;

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






