/*-----------------------------------------------------------------------

File  : cpr_dpll.h

Author: Stephan Schulz

Contents

  Definitions for the main DPLL algorithm.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue May  6 02:04:46 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef CPR_DPLL

#define CPR_DPLL

#include <cpr_varset.h>
#include <cpr_dpllformula.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef struct dpll_state_cell
{
   DPLLFormula_p form;
   PStack_p      assignment; /* Current assignment, atom code
                                represents positive assignment, -atom
                                code represents negative assignment */
   PStack_p      deactivated; /* Clauses that have been deactivated,
             each initiated by a NULL pointer */
   PStack_p      unproc_units; /* Known unprocessed unit clauses */
   AtomSet_p     open_atoms;  /* List of unassigned variables */
}DPLLStateCell, *DPLLState_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define DPLLStateCellAlloc() (DPLLStateCell*)SizeMalloc(sizeof(DPLLStateCell))
#define DPLLStateCellFree(junk)            SizeFree(junk, sizeof(DPLLStateCell))

DPLLState_p DPLLStateAlloc(DPLLFormula_p form);
void        DPLLStateFree(DPLLState_p junk);

bool      DPLLAssignVar(DPLLState_p state, PLiteralCode assignment);
void      DPLLRetractLastAss(DPLLState_p state);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





