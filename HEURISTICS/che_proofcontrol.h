/*-----------------------------------------------------------------------

File  : che_proofcontrol.h

Author: Stephan Schulz

Contents

  Object storing all information about control of the search
  process: Ordering, heuristic, similar stuff.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Oct 16 14:52:53 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_PROOFCONTROL

#define CHE_PROOFCONTROL

#include <ccl_rewrite.h>
#include <ccl_proofstate.h>
#include <che_hcbadmin.h>
#include <che_to_weightgen.h>
#include <che_to_precgen.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


typedef struct proofcontrolcell
{
   OCB_p               ocb;
   HCB_p               hcb;
   WFCBAdmin_p         wfcbs;
   HCBAdmin_p          hcbs;
   bool                ac_handling_active;
   HeuristicParmsCell  heuristic_parms;
   FVIndexParmsCell    fvi_parms;
   SpecFeatureCell     problem_specs;
   /* Sat solver object. */
   SatSolver_p         solver;
}ProofControlCell, *ProofControl_p;

#define HCBARGUMENTS ProofState_p state, ProofControl_p control, \
                     HeuristicParms_p parms

typedef HCB_p (*HCBCreateFun)(HCBARGUMENTS);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

extern  char* DefaultWeightFunctions;
extern  char* DefaultHeuristics;

#define ProofControlCellAlloc() \
   (ProofControlCell*)SizeMalloc(sizeof(ProofControlCell))
#define ProofControlCellFree(junk) \
   SizeFree(junk, sizeof(ProofControlCell))

ProofControl_p ProofControlAlloc(void);
void           ProofControlFree(ProofControl_p junk);
void           ProofControlResetSATSolver(ProofControl_p ctrl);

void           DoLiteralSelection(ProofControl_p control, Clause_p
              clause);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
