/*-----------------------------------------------------------------------

File  : cco_ho_inferences.h

Author: Petar Vukmirovic

Contents

  Declarations of functions that implement higher-order inferences that are
  non-essential to superposition. 

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

-----------------------------------------------------------------------*/

#include <che_proofcontrol.h>

bool ImmediateClausification(Clause_p cl, ClauseSet_p store, 
                             ClauseSet_p archive, VarBank_p fresh_vars,
                             bool fool_unroll);
bool ResolveFlexClause(Clause_p cl);
bool NormalizeEquations(Clause_p cl);
void ComputeHOInferences(ProofState_p state, ProofControl_p control, 
                         Clause_p renamed_clause, Clause_p orig_clause);
bool BooleanSimplification(Clause_p cl);
void ClausePruneArgs(Clause_p cl);
void PreinstantiateInduction(FormulaSet_p forms, ClauseSet_p cls, ClauseSet_p archive, TB_p bank);
void ClauseSetRecognizeChoice(IntMap_p choice_syms, ClauseSet_p set, 
                              ClauseSet_p archive);
