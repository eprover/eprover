/*-----------------------------------------------------------------------

  File  : che_wfcb.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Weigth-function-Control blocks, functions computing weights for
  clauses.

  The interface to an evaluation function requires 3 or 4 functions:

  WFCB_p <eval>Init(PrioFun prio, &rest)

  This function takes a prority function and optional arguments,
  and return a WFCB. In particular, it is responsible for creating
  the data block of the WFCB. Resonable additional arguments are
  the OCB, the signature, or the set of axioms.

  WFCB_p <eval>Parse(Scanner_p in, OCB_p ocb, ProofState_p state)

  Extracts the arguments from an input stream and passes then on to
  the init-function. ocb contains an initialized OCB, state is a
  partially initialized proof state: Only state->axioms has to be
  initialized. However, state is guaranteed to be the state used in
  saturating (i.e. the weight functions can access the fully
  initialized state later on).

  double <eval>Compute(void *data, Clause_p clause)

  Given a clause and a data block, return an evaluation for the
  clause.

  void <Eval>Exit(void* data)

  This function is responsible for freeing data, before the WFCB is
  deleted.

  Copyright 1998-2018 by the authors (see DOC/CONTRIBUTORS).
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Fri Jun  5 21:39:40 MET DST 1998

  -----------------------------------------------------------------------*/

#ifndef CHE_WFCB

#define CHE_WFCB

#include <clb_dstacks.h>
#include <cio_output.h>
#include <ccl_proofstate.h>
#include <che_prio_funs.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Weight functions and their data */

typedef double (*ClauseEvalFun)(void* data, Clause_p
                                clause);

typedef struct wfcb_cell
{
   ClauseEvalFun     wfcb_eval;     /* Compute a clauses evaluation */
   GenericExitFun    wfcb_exit;     /* Clean up - in particular, free
                                       data */
   ClausePrioFun     wfcb_priority; /* Compute the priority */
   void*             data;          /* WFCB-Data...each set of
                                       evaluation functions is
                                       responsible for cleaning up...*/
}WFCBCell, *WFCB_p;

typedef WFCB_p (*WeightFunParseFun)(Scanner_p in, OCB_p ocb,
                                    ProofState_p state);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define WFCBCellAlloc() (WFCBCell*)SizeMalloc(sizeof(WFCBCell))
#define WFCBCellFree(junk)         SizeFree(junk, sizeof(WFCBCell))

WFCB_p WFCBAlloc(ClauseEvalFun wfcb_eval, ClausePrioFun prio_fun,
                 GenericExitFun wfcb_exit, void* data);
void   WFCBFree(WFCB_p junk);

void   ClauseAddEvaluation(WFCB_p wfcb, Clause_p clause, int pos, bool empty);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
