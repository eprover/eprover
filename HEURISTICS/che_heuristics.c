/*-----------------------------------------------------------------------

File  : che_heuristics.c

Author: Stephan Schulz

Contents
 
  High-Level interface functions to the heuristics module.


  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun  8 02:14:51 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_heuristics.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


HeuristicAssocCell HeuristicsTable[]=
{
   {HEU_AUTO_MODE,     "Auto",      HCBAutoModeCreate},
   {HEU_AUTO_MODE_071, "Auto071",   HCB071AutoModeCreate},
   {HEU_AUTO_MODE_DEV, "AutoDev",   HCBDevAutoModeCreate},
#ifdef SAFELOGIC
   {HEU_SL_AUTO_MODE,  "SLAuto",    HCBSLAutoModeCreate},
#endif
   {HEU_NO_HEURISTIC, NULL,         (HCBCreateFun)NULL}
};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

#define CHE_HEURISTICS_INTERNAL

/*-----------------------------------------------------------------------
//
// Function: HCBAutoModeCreate()
//
//   Analyse the proof problem and return an hopefully suitable
//   heuristic. 
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO
HCB_p HCBAutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits =  SpecLimitsAlloc();

   limits->ax_some_limit        = 46;
   limits->ax_many_limit        = 205;
   limits->lit_some_limit       = 212;
   limits->lit_many_limit       = 620;
   limits->term_medium_limit    = 163;
   limits->term_large_limit     = 2270;
   limits->far_sum_medium_limit = 3;
   limits->far_sum_large_limit  = 75;

   control->selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
   
#include "che_auto_cases.c"

   if(OutputLevel)
   {
      fprintf(stdout, 
	      "# Auto-Mode selected heuristic %s\n"
	      "# and selection function %s.\n#\n", res, 
	      GetLitSelName(control->selection_strategy)); 
   }
   if(parms->mem_limit>2 && (parms->delete_bad_limit ==
			     DEFAULT_DELETE_BAD_LIMIT))
   {
      control->delete_bad_limit =
	 (float)(parms->mem_limit-2)*0.7*MEGA;
      /* control->filter_copies_limit = control->delete_bad_limit*0.7; */
   }
   if(SpecNoEq(spec))
   {
      control->ac_handling = NoACHandling;
      OUTPRINT(1, "# No equality, disabling AC handling.\n#\n");
   }
   SpecLimitsCellFree(limits);
   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO

/*-----------------------------------------------------------------------
//
// Function: HCB071AutoModeCreate()
//
//   Analyse the proof problem and return an hopefully suitable
//   heuristic, using the criteria from E 0.71, the last version
//   without contextual cutting.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO_071
HCB_p HCB071AutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits =  SpecLimitsAlloc();

   control->selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
   
#include "che_auto_cases.c"

   if(OutputLevel)
   {
      fprintf(stdout, 
	      "# Auto-Mode (0.71) selected heuristic %s\n"
	      "# and selection function %s.\n#\n", res, 
	      GetLitSelName(control->selection_strategy)); 
   }
   if(parms->mem_limit>2 && (parms->delete_bad_limit ==
			     DEFAULT_DELETE_BAD_LIMIT))
   {
      control->delete_bad_limit =
	 (float)(parms->mem_limit-2)*0.7*MEGA;
      /* control->filter_copies_limit = control->delete_bad_limit*0.7; */
   }
   if(SpecNoEq(spec))
   {
      control->ac_handling = NoACHandling;
      OUTPRINT(1, "# No equality, disabling AC handling.\n#\n");
   }
   SpecLimitsCellFree(limits);
   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_071


/*-----------------------------------------------------------------------
//
// Function: HCBDevAutoModeCreate()
//
//   Analyse the proof problem and return an hopefully suitable
//   heuristic, using the latest development auto mode.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/


#define CHE_HEURISTICS_AUTO_DEV
HCB_p HCBDevAutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits =  SpecLimitsAlloc();

   limits->ax_some_limit        = 46;
   limits->ax_many_limit        = 205;
   limits->lit_some_limit       = 212;
   limits->lit_many_limit       = 620;
   limits->term_medium_limit    = 163;
   limits->term_large_limit     = 2270;
   limits->far_sum_medium_limit = 3;
   limits->far_sum_large_limit  = 75;

   control->selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Auto-Heuristic is analysing problem.\n");
   
#include "che_auto_cases.c"

   if(OutputLevel)
   {
      fprintf(stdout, 
	      "# Auto-Mode (Dev) selected heuristic %s\n"
	      "# and selection function %s.\n#\n", res, 
	      GetLitSelName(control->selection_strategy)); 
   }
   if(parms->mem_limit>2 && (parms->delete_bad_limit ==
			     DEFAULT_DELETE_BAD_LIMIT))
   {
      control->delete_bad_limit =
	 (float)(parms->mem_limit-2)*0.7*MEGA;
      /* control->filter_copies_limit = control->delete_bad_limit*0.7; */
   }
   if(SpecNoEq(spec))
   {
      control->ac_handling = NoACHandling;
      OUTPRINT(1, "# No equality, disabling AC handling.\n#\n");
   }
   SpecLimitsCellFree(limits);
   return GetHeuristic(res, state, control, parms);
}
#undef CHE_HEURISTICS_AUTO_DEV

#ifdef SAFELOGIC
/*-----------------------------------------------------------------------
//
// Function: HCBSLAutoModeCreate()
//
//   As above, but use partially proprietary techniques sponsored by
//   Safelogic A.B.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_INTERNAL

HCB_p HCBSLAutoModeCreate(HCBARGUMENTS)
{
   char *res = "Default";
   SpecFeature_p spec = &(control->problem_specs);
   SpecLimits_p limits =  SpecLimitsAlloc();

   control->selection_strategy = SelectNoLiterals;
   OUTPRINT(1, "# Safelogic-Auto-Heuristic is analysing problem.\n");
   
#include "../SAFELOGIC/csl_auto_cases.c"

   if(OutputLevel)
   {
      fprintf(stdout, 
	      "# Safelogic-Auto-Mode selected heuristic %s\n"
	      "# and selection function %s.\n#\n", res, 
	      GetLitSelName(control->selection_strategy)); 
   }
   if(parms->mem_limit>2 && (parms->delete_bad_limit ==
			     DEFAULT_DELETE_BAD_LIMIT))
   {
      control->delete_bad_limit =
	 (float)(parms->mem_limit-2)*0.7*MEGA;
      /* control->filter_copies_limit = control->delete_bad_limit*0.7; */
   }
   if(SpecNoEq(spec))
   {
      control->ac_handling = NoACHandling;
      OUTPRINT(1, "# No equality, disabling AC handling.\n#\n");
   }
   SpecLimitsCellFree(limits);
   return GetHeuristic(res, state, control, parms);
}
#endif


/*-----------------------------------------------------------------------
//
// Function: HCBCreate() 
//
//   Given a name and the necessary parameters, return a HCB. HCB's
//   must be registered in control->hcbs!
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

HCB_p HCBCreate(char* name, HCBARGUMENTS)
{
   int    i;
   DStr_p err;

   assert(control->ocb);

   for(i=0; HeuristicsTable[i].heuristic; i++)
   {
      if(strcmp(HeuristicsTable[i].name, name)==0)
      {
	 return HeuristicsTable[i].heuristic_create(state,control,parms);
      }
   }
   err = DStrAlloc();
   DStrAppendStr(err, "Heuristic \"");
   DStrAppendStr(err, name);
   DStrAppendStr(err, "\" unknown");
   Error(DStrView(err), USAGE_ERROR);
   DStrFree(err);
   
   return NULL;
}


/*-----------------------------------------------------------------------
//
// Function: GetHeuristic()
//
//   Given a string (either a name or a Heuristic-Definition), return
//   a corresponding HCB.
//
// Global Variables: -
//
// Side Effects    : May create HCB's and WFCB's
//
/----------------------------------------------------------------------*/

HCB_p GetHeuristic(char* source, HCBARGUMENTS)
{
   char*     name;
   Scanner_p in;
   HCB_p     res;

   /* StreamTypeOptionString is a hack here...it will give wrong error
      messages for internal strings (but the function will not be
      called with internal strings anyways...) */
   in=CreateScanner(StreamTypeOptionString, source, true, NULL);
   if(TestInpTok(in, OpenBracket))
   {
      HeuristicDefParse(control->hcbs, in, control->wfcbs,
			control->ocb, state);
      name = SecureStrdup("Default");
   }
   else
   {      
      name = SecureStrdup(DStrView(AktToken(in)->literal));
      AcceptInpTok(in, Identifier);
   }
   DestroyScanner(in);
   res = HCBAdminFindHCB(control->hcbs, name);
   if(!res)
   {
      res = HCBCreate(name, state, control, parms);
      assert(res);
   }
   FREE(name);
   return res;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


