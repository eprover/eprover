/*-----------------------------------------------------------------------

  File  : ccl_satinterface.h

  Author: Stephan Schulz

  Contents

  Datatypes and declarations for efficient conversion of the proof
  state to propositional clauses and submission to a SAT solver.

  Copyright 2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Sep 15 20:29:49 CEST 2017

  -----------------------------------------------------------------------*/

#ifndef CCL_SATINTERFACE

#define CCL_SATINTERFACE

#include <ccl_proofstate.h>
#include <cio_tempfile.h>
#include <picosat.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



typedef struct satclausecell
{
   bool       has_pure_lit;
   int        lit_no;
   int *      literals;
   Clause_p   source;
}SatClauseCell, *SatClause_p;

typedef struct satclausesetcell
{
   PDRangeArr_p renumber_index; // Used to map term->id to [0...max_lit]
   int          max_lit;
   PStack_p     set;            // Actual set (clauses must be freed)
   PStack_p     printset;       // Subset last printed, references
                                // only
   long         core_size;      // Size of the unsat core, if any
}SatClauseSetCell, *SatClauseSet_p;


typedef enum
{
   GMNoGrounding,
   GMPseudoVar,
   GMFirstConst,
   GMConjMinMinFreq, /* Rarest conjecture constant */
   GMConjMaxMinFreq,
   GMConjMinMaxFreq, /* Rarest in conjectures, most frequent overall */
   GMConjMaxMaxFreq,
   GMGlobalMax,
   GMGlobalMin
}GroundingStrategy;


typedef PicoSAT* SatSolver_p;

#define SAT_TO_E_RESULT(satres) ((satres) == PICOSAT_SATISFIABLE ?\
                                  PRSatisfiable : (assert((satres) == PICOSAT_UNSATISFIABLE),\
                                                  PRUnsatisfiable))
typedef bool (*SatClauseFilter)(SatClause_p);

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern char* GroundingStratNames[];

#define SatClauseCellAlloc()    (SatClauseCell*)SizeMalloc(sizeof(SatClauseCell))
#define SatClauseCellFree(junk) SizeFree(junk, sizeof(SatClauseCell))

SatClause_p SatClauseAlloc(int lit_no);
void        SatClauseFree(SatClause_p junk);

#define SatClauseSetCellAlloc()    (SatClauseSetCell*)SizeMalloc(sizeof(SatClauseSetCell))
#define SatClauseSetCellFree(junk) SizeFree(junk, sizeof(SatClauseSetCell))


SatClauseSet_p SatClauseSetAlloc(void);
void           SatClauseSetFree(SatClauseSet_p junk);

#define SatClauseSetCardinality(satset) PStackGetSP((satset)->set)
#define SatClauseSetNonPureCardinality(satset) PStackGetSP((satset)->printset)
#define SatClauseSetCoreSize(satset) (satset)->core_size


SatClause_p SatClauseCreateAndStore(Clause_p clause, SatClauseSet_p set);
void        SatClausePrint(FILE* out, SatClause_p satclause);

void        SatClauseSetPrint(FILE* out, SatClauseSet_p set);
void        SatClauseSetPrintNonPure(FILE* out, SatClauseSet_p set, long pure);

Subst_p     SubstPseudoGroundVarBank(VarBank_p vars);
Subst_p     SubstGroundVarBankFirstConst(TB_p terms, bool norm_const);
Subst_p     SubstGroundFreqBased(TB_p terms, ClauseSet_p clauses,
                                 FunConstCmpFunType is_better, bool norm_const);

long        SatClauseSetImportClauseSet(SatClauseSet_p satset, ClauseSet_p set);
long        SatClauseSetImportProofState(SatClauseSet_p satset, ProofState_p state,
                                         GroundingStrategy strat, bool norm_const);
long        SatClauseSetMarkPure(SatClauseSet_p satset);
ProverResult SatClauseSetCheckUnsat(SatClauseSet_p satset, Clause_p *empty,
                                    SatSolver_p solver,
                                    int sat_check_decision_level);



#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
