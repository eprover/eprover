/*-----------------------------------------------------------------------

File  : cpr_dpll.c

Author: Stephan Schulz

Contents

  Code for the main DPLL algorithm.

Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed May 14 01:56:29 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "cpr_dpll.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: deactivate_clauses()
//
//   Deactivete all clauses in *tree and record them on
//   state->decativated. Return number of clauses.
//
// Global Variables: -
//
// Side Effects    : Changes state.
//
/----------------------------------------------------------------------*/

long deactivate_clauses(DPLLState_p state, PTree_p *clauses)
{
   long res = 0;

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: shorten_clauses()
//
//   Shorten all clauses in *tree by one.
//
// Global Variables: -
//
// Side Effects    : Changes state
//
/----------------------------------------------------------------------*/

long shorten_clauses(DPLLState_p state, PTree_p *clauses)
{
   long res = 0;

   return res;
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: DPLLStateAlloc()
//
//   Allocate an initialized DPLL search state.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

DPLLState_p DPLLStateAlloc(DPLLFormula_p form)
{
   DPLLState_p handle = DPLLStateCellAlloc();
   long i, limit;
   DPLLClause_p clause;

   handle->form         = form;
   handle->assignment   = PStackAlloc();
   handle->deactivated  = PStackAlloc();
   handle->unproc_units = PStackAlloc();
   handle->open_atoms   = AtomSetAlloc();

   for(i=1; i<form->atom_no; i++)
   {
      if(form->atoms[i].pos_occur + form->atoms[i].neg_occur)
      {
    AtomSetInsert(handle->open_atoms, i);
      }
   }
   limit = PStackGetSP(form->clauses);
   for(i=0; i<limit; i++)
   {
      clause = PStackElementP(form->clauses, i);
      if(DPLLClauseIsUnit(clause))
      {
    PStackPushP(handle->unproc_units, clause);
      }
   }
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: DPLLStateFree()
//
//   Free a DPLL search state
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DPLLStateFree(DPLLState_p junk)
{
   PStackFree(junk->assignment);
   PStackFree(junk->deactivated);
   PStackFree(junk->unproc_units);
   AtomSetFree(junk->open_atoms);
   DPLLFormulaFree(junk->form);
   DPLLStateCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: DPLLAssignVar()
//
//   Extend the assignment with the given new propositional variable
//   assignment. Return true if no empty clause has been generated.
//
// Global Variables: -
//
// Side Effects    : Changes state!
//
/----------------------------------------------------------------------*/

bool DPLLAssignVar(DPLLState_p state, PLiteralCode assignment)
{
   bool res = true;
   Atom_p atom;

   PStackPushInt(state->assignment, assignment);
   PStackPushP(state->deactivated, NULL); /* Mark new subset */

   if(assignment > 0)
   {
      atom = &(state->form->atoms[assignment]);
      deactivate_clauses(state, &(atom->pos_active));
      res = shorten_clauses(state, &(atom->neg_active));
   }
   else
   {
      assignment = -assignment;
      atom = &(state->form->atoms[assignment]);
      deactivate_clauses(state, &(atom->pos_active));
      res = shorten_clauses(state, &(atom->neg_active));
   }
   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/