/*-----------------------------------------------------------------------

File  : che_proofcontrol.c

Author: Stephan Schulz

Contents

  Basic functions for proof control objects.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Oct 16 16:20:00 MET DST 1998
    Salvaged from cco_proofstate.h, forked control and state.
<2> Wed Dec 16 18:45:14 MET 1998
    Moved from cco to che

-----------------------------------------------------------------------*/

#include "che_proofcontrol.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

#define CHE_PROOFCONTROL_INTERNAL

char* DefaultWeightFunctions =
"\n"
"weight21_ugg  = Clauseweight(PreferUnitGroundGoals,2,1,1)      \n"
"rweight21_g   = Refinedweight(PreferGoals,    2,1,1.5,1.1,1) \n"
"rweight11_g   = Refinedweight(PreferGoals,    1,1,1.5,1.1,1.1) \n"
"rweight21_a   = Refinedweight(PreferNonGoals, 2,1,1.5,1.1,1.1) \n"
"rweight21_ugg = Refinedweight(PreferUnitGroundGoals, 2,1,1.5,1.1,1.1) \n"
"fifo_f        = FIFOWeight(ConstPrio)                      \n"
"lifo_f        = LIFOWeight(ConstPrio)                      \n"
"weight11_f    = Clauseweight(ConstPrio,1,1,1)              \n"
"weight11_ugg  = Clauseweight(PreferUnitGroundGoals,1,1,1)  \n"
"weight21_f    = Clauseweight(ConstPrio,2,1,1)              \n"
"TSMRDefault   = TSMWeight(ConstPrio, 1, 1, 2, flat, E_KNOWLEDGE,"
                 "100000,1.0,1.0,Flat,IndexIdentity,100000,-20,20,-2,-1,0,2)\n"
;

char* DefaultHeuristics=
"Weight     = (1*weight21_ugg)                       \n"
"WeightC1   = (1*weight11_ugg)                       \n"
"StandardWeight = (1*weight21_f)                     \n"
"StandardPG = (5*weight21_f,1*fifo_f)                \n"
"RWeight    = (1*rweight21_ugg)                      \n"
"FIFO       = (1*fifo_f)                             \n"
"LIFO       = (1*lifo_f)                             \n"
"Default    = (3*rweight21_a, 1*rweight21_g)         \n"
"Uniq       = (1*Uniqweight(ConstPrio))\n"
"UseWatchlist = \n"
"(10*Refinedweight(PreferGoals,1,2,2,2,0.5),"
" 10*Refinedweight(PreferNonGoals,2,1,2,2,2),"
" 5*OrientLMaxWeight(PreferWatchlist,2,1,2,1,1),"
" 1*FIFOWeight(PreferWatchlist))\n"
"UseWatchlistPure=\n"
"(1*Defaultweight(PreferWatchlist))\n"
"UseWatchlistPG10=\n"
"(10*Defaultweight(PreferWatchlist),\n"
" 1*FIFOWeight(ConstPrio))\n"
"UseWatchlistEvo=\n"
"(1*ConjectureRelativeSymbolWeight(SimulateSOS,0.5, 100, 100,\n"
"                                  100, 100, 1.5, 1.5, 1),\n"
" 4*ConjectureRelativeSymbolWeight(PreferWatchlist,0.1, 100, \n"
"                                  100, 100, 100, 1.5, 1.5, 1.5),\n"
" 1*FIFOWeight(PreferProcessed),\n"
" 1*ConjectureRelativeSymbolWeight(PreferWatchlist,0.5, 100, 100, \n"
"                                  100, 100, 1.5, 1.5, 1),\n"
" 4*Refinedweight(SimulateSOS,3,2,2,1.5,2))\n"
"UseTSM1 = \n"
"(10*Refinedweight(PreferGoals,1,2,2,2,0.5),"
" 10*Refinedweight(PreferNonGoals,2,1,2,2,2),"
" 5*TSMRDefault,"
" 1*FIFOWeight(PreferWatchlist))\n"
"UseTSM2 = \n"
"(20*TSMRDefault,"
" 5*OrientLMaxWeight(PreferWatchlist,2,1,2,1,1),"
" 1*FIFOWeight(PreferWatchlist))."
;
/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: select_inherited_literal()
//
//   If there is at least one negative literal with EPIsPMIntoLit,
//   select all literals with this property, return true. Otherwise
//   return false.
//
// Global Variables: -
//
// Side Effects    : Changes literal properties.
//
/----------------------------------------------------------------------*/

static bool select_inherited_literal(Clause_p clause)
{
   Eqn_p handle;
   bool  found  = false;

   if(clause->neg_lit_no == 0)
   {
      return false;
   }
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsNegative(handle)&&EqnQueryProp(handle,EPIsPMIntoLit))
      {
         found = true;
         break;
      }
   }
   if(!found)
   {
      return false;
   }
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnQueryProp(handle,EPIsPMIntoLit))
      {
         EqnSetProp(handle, EPIsSelected);
      }
   }
   return true;
}

/*-----------------------------------------------------------------------
//
// Function: sat_solver_init()
//
//   Create and initialize the SAT solver in the ProofControl object.
//
// Global Variables: -
//
// Side Effects    : Via PicoSAT interface
//
/----------------------------------------------------------------------*/

void sat_solver_init(ProofControl_p ctrl)
{
   ctrl->solver = picosat_init();
#ifndef NDEBUG
   int status =
#endif
   picosat_enable_trace_generation(ctrl->solver);
   assert(status);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: ProofControlAlloc()
//
//   Allocate an empty, initialized ProofControlCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

ProofControl_p ProofControlAlloc(void)
{
   ProofControl_p handle = ProofControlCellAlloc();
   handle->ocb                           = NULL;
   handle->wfcbs                         = WFCBAdminAlloc();
   handle->hcbs                          = HCBAdminAlloc();
   handle->hcb                           = NULL;
   handle->ac_handling_active            = false;
   HeuristicParmsInitialize(&handle->heuristic_parms);

   sat_solver_init(handle);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ProofControlFree()
//
//   Free a ProofControlCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ProofControlFree(ProofControl_p junk)
{
   assert(junk);
   if(junk->ocb)
   {
      OCBFree(junk->ocb);
   }
   WFCBAdminFree(junk->wfcbs);
   HCBAdminFree(junk->hcbs);
   /* hcb is always freed in junk->hcbs */

   if(junk->solver)
   {
      picosat_reset(junk->solver);
   }
   ProofControlCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: ProofContrlResetSATSolver()
//
//   Resets SAT solver state to make it ready for the next attempt.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ProofControlResetSATSolver(ProofControl_p ctrl)
{
   picosat_reset(ctrl->solver);
   sat_solver_init(ctrl);
}



/*-----------------------------------------------------------------------
//
// Function: DoLiteralSelection()
//
//   Based on control, select a literal selection strategy and apply
//   it to clause.
//
// Global Variables: -
//
// Side Effects    : Changes properties in clause
//
/----------------------------------------------------------------------*/

void DoLiteralSelection(ProofControl_p control, Clause_p clause)
{
   EqnListDelProp(clause->literals, EPIsSelected);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
   ClauseDelProp(clause, CPIsOriented);
   assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);

   if(control->heuristic_parms.inherit_paramod_lit||
      (control->heuristic_parms.inherit_goal_pm_lit&&ClauseIsGoal(clause))||
      (control->heuristic_parms.inherit_conj_pm_lit&&ClauseIsConjecture(clause))
      )
   {
      if(select_inherited_literal(clause))
      {
    return;
      }
   }
   if(clause->neg_lit_no &&
      (clause->pos_lit_no >= control->heuristic_parms.pos_lit_sel_min) &&
      (clause->pos_lit_no <= control->heuristic_parms.pos_lit_sel_max) &&
      (clause->neg_lit_no >= control->heuristic_parms.neg_lit_sel_min) &&
      (clause->neg_lit_no <= control->heuristic_parms.neg_lit_sel_max) &&
      (ClauseLiteralNumber(clause) >= control->heuristic_parms.all_lit_sel_min) &&
      (ClauseLiteralNumber(clause) <= control->heuristic_parms.all_lit_sel_max) &&
      ((control->heuristic_parms.weight_sel_min==0) || /* Efficiency hack - only
                 compute clause weight if this
                 option is activated */
       (control->heuristic_parms.weight_sel_min<=ClauseStandardWeight(clause))))
   {
      assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
      control->heuristic_parms.selection_strategy(control->ocb,clause);
   }
   else
   {
      assert(EqnListQueryPropNumber(clause->literals, EPIsSelected)==0);
      SelectNoLiterals(control->ocb,clause);
   }
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
