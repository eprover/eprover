/*-----------------------------------------------------------------------

  File  : cco_forward_contraction.h

  Author: Stephan Schulz

  Contents

  Functions that apply the processed clause sets to simplify or
  eliminate a potential new clause. Extracted from
  cco_proofproc.[ch].

  Copyright 1998-2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created Mon Nov  9 17:46:50 MET 1998

  -----------------------------------------------------------------------*/

#ifndef CCO_FORWARD_CONTRACTION

#define CCO_FORWARD_CONTRACTION

#include <cio_output.h>
#include <cco_simplification.h>
#include <cco_factoring.h>
#include <cco_eqnresolving.h>
#include <cco_paramodulation.h>
#include <ccl_tautologies.h>
#include <ccl_context_sr.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DEFAULT_FILTER_DESCRIPTOR "Fc"


FVPackedClause_p ForwardSubsumption(ProofState_p state,
                                    Clause_p clause,
                                    unsigned long* subsumed_count,
                                    bool non_unit_subsumption);

bool ForwardModifyClause(ProofState_p state,
                         ProofControl_p control,
                         Clause_p clause,
                         bool context_sr,
                         bool condense,
                         RewriteLevel level);

FVPackedClause_p ForwardContractClause(ProofState_p state,
                                       ProofControl_p control,
                                       Clause_p clause,
                                       bool non_unit_subsumption,
                                       bool context_sr,
                                       bool condense,
                                       RewriteLevel level);

Clause_p ForwardContractSet(ProofState_p state, ProofControl_p
                            control, ClauseSet_p set, bool
                            non_unit_subsumption, RewriteLevel level,
                            unsigned long* count_eliminated, bool
                            terminate_on_empty);

void     ClauseSetReweight(HCB_p heuristic, ClauseSet_p set);

Clause_p ForwardContractSetReweight(ProofState_p state, ProofControl_p
                                    control, ClauseSet_p set, bool
                                    non_unit_subsumption, RewriteLevel
                                    level, unsigned long* count_eliminated);

void     ClauseSetFilterReweigth(ProofControl_p control, ClauseSet_p
                                 set, unsigned long* count_eliminated);

Clause_p ProofStateFilterUnprocessed(ProofState_p state,
                                     ProofControl_p control, char*
                                     desc);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
