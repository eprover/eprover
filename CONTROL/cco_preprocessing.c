/*-----------------------------------------------------------------------

  File  : cco_preprocessing.c

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  This module encapsulates some of the main proofstate preprocessing,
  mostly to keep the complexity of eprover.c under control.

  Copyright 2025 by the authors.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

  Created:

  -----------------------------------------------------------------------*/

#include "cco_preprocessing.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ProofStateIntroduceGroundTermDefs()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long ProofStateIntroduceGroundTermDefs(ProofState_p proofstate,
                                       HeuristicParms_p h_parms)
{
   long res = 0;
   Clause_p handle;
   PTree_p terms_to_define;

   printf("ProofStateIntroduceGroundTermDefs()...\n");

   for(handle = proofstate->axioms->anchor->succ;
       handle!=proofstate->axioms->anchor;
       handle = handle->succ)
   {
      if(ClauseIsConjecture(handle))
      {
         ClauseCollectGroundTerms(handle, &terms_to_define,
                                  h_parms->add_goal_defs_subterms,
                                  h_parms->add_goal_defs_pos,
                                  h_parms->add_goal_defs_neg);
      }
   }

   printf("...ProofStateIntroduceGroundTermDefs()\n");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ProofStateClausalPreproc()
//
//   Perform various (optional) preprocessing steps on the proof state
//   unprocessed clauses.
//
// Global Variables: -
//
// Side Effects    : Archives original clauses, may modify a lot of
//                   the proof state in various ways
//
/----------------------------------------------------------------------*/

long ProofStateClausalPreproc(ProofState_p proofstate,
                              HeuristicParms_p h_parms)
{
   long preproc_removed = 0;

   ClauseSetArchiveCopy(proofstate->ax_archive, proofstate->axioms);
   if(!h_parms->no_preproc)
   {
      VERBOUT("Clausal preprocessing started.\n");
      preproc_removed = ClauseSetPreprocess(proofstate->axioms,
                                            proofstate->watchlist,
                                            proofstate->archive,
                                            proofstate->tmp_terms,
                                            proofstate->terms,
                                            h_parms->replace_inj_defs,
                                            h_parms->eqdef_incrlimit,
                                            h_parms->eqdef_maxclauses);
      VERBOUT("Clausal preprocessing complete.\n");
   }

   preproc_removed += ClauseSetUnfoldEqDefNormalize(proofstate->axioms,
                                                    proofstate->watchlist,
                                                    proofstate->archive,
                                                    proofstate->tmp_terms,
                                                    h_parms->eqdef_incrlimit,
                                                    h_parms->eqdef_maxclauses);

   if(problemType == PROBLEM_HO && h_parms->inst_choice_max_depth >= 0)
   {
      ClauseSetRecognizeChoice(proofstate->choice_opcodes,
                               proofstate->axioms,
                               proofstate->archive);
   }

   if(h_parms->preinstantiate_induction)
   {
      PreinstantiateInduction(proofstate->f_ax_archive, proofstate->axioms,
                              proofstate->archive, proofstate->terms);
   }

   if(problemType == PROBLEM_FO && h_parms->bce)
   {
      // todo: eventually check if the problem in HO syntax is FO.
      EliminateBlockedClauses(proofstate->axioms, proofstate->archive,
                              h_parms->bce_max_occs,
                              proofstate->tmp_terms);
   }

   if(problemType == PROBLEM_FO && h_parms->pred_elim)
   {
      // todo: eventually check if the problem in HO syntax is FO.
      PredicateElimination(proofstate->axioms, proofstate->archive,
                           h_parms, proofstate->terms,
                           proofstate->tmp_terms, proofstate->freshvars);
   }
   return preproc_removed;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
