/*-----------------------------------------------------------------------

File  : che_proofcontrol.h

Author: Stephan Schulz

Contents
 
  Object storing all information about control of the search
  process: Ordering, heuristic, similar stuff.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Fri Oct 16 14:52:53 MET DST 1998
    New

-----------------------------------------------------------------------*/

#ifndef CHE_PROOFCONTROL

#define CHE_PROOFCONTROL

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
   RewriteLevel        forward_demod;
   bool                prefer_general;
   long                filter_limit;
   long                filter_copies_limit;
   long                reweight_limit;
   long                delete_bad_limit;
   ACHandlingType      ac_handling;  
   bool                ac_res_aggressive;
   bool                ac_handling_active;
   bool                er_varlit_destructive;
   bool                er_strong_destructive;
   bool                er_aggressive;
   bool                forward_context_sr;
   bool                forward_context_sr_aggressive;
   bool                backward_context_sr;
   bool                prefer_initial_clauses;
   bool                select_on_proc_only;
   bool                inherit_paramod_lit;
   bool                inherit_goal_pm_lit;
   LiteralSelectionFun selection_strategy;
   long                pos_lit_sel_min;
   long                pos_lit_sel_max;
   long                neg_lit_sel_min;
   long                neg_lit_sel_max;
   long                all_lit_sel_min;
   long                all_lit_sel_max;
   long                weight_sel_min;
   SplitClassType      split_clauses;
   SplitType           split_method;
   bool                split_aggressive;
   UnitSimplifyType    unproc_simplify;
   bool                watchlist_simplify;
   SpecFeatureCell     problem_specs;
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

void           DoLiteralSelection(ProofControl_p control, Clause_p
				  clause);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/






