/*-----------------------------------------------------------------------

File  : che_wfcb.c

Author: Stephan Schulz

Contents
 
  Functions for evaluation function control blocks.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun  5 22:05:39 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_wfcb.h"



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
// Function: WFCBAlloc()
//
//   Create and return an initialized WFCB-block.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p WFCBAlloc(ClauseEvalFun wfcb_eval, ClausePrioFun prio_fun,
		 GenericExitFun wfcb_exit, void* data)
{
   WFCB_p handle = WFCBCellAlloc();

   handle->wfcb_eval = wfcb_eval;
   handle->wfcb_priority = prio_fun;
   handle->wfcb_exit = wfcb_exit;
   handle->data = data;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: WFCBFree()
//
//   Free a WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void WFCBFree(WFCB_p junk)
{
   assert(junk);
   
   if(junk->data)
   {
      junk->wfcb_exit(junk->data);
   }
   WFCBCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseAddEvaluation()
//
//   Given a clause and a wfcb, add an evaluation to the clause.
//
// Global Variables: -
//
// Side Effects    : Adds evaluation, by calling wfcb->compute_eval()
//
/----------------------------------------------------------------------*/

void ClauseAddEvaluation(WFCB_p wfcb, Clause_p clause)
{
   Eval_p eval = EvalAlloc();
   
   eval->heuristic = wfcb->wfcb_eval(wfcb->data, clause);
   eval->priority  = wfcb->wfcb_priority(clause);
   ClauseAddEvalCell(clause, eval);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


