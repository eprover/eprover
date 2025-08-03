/*-----------------------------------------------------------------------

  File  : che_heuristics.c

  Author: Stephan Schulz

  Contents

  High-Level interface functions to the heuristics module.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon Jun  8 02:14:51 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_heuristics.h"
#include "che_new_autoschedule.h"



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
// Function: finalize_auto_parms()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void finalize_auto_parms(char* modename, char* hname,
                         ProofControl_p control,
                         HeuristicParms_p parms,
                         SpecFeature_p spec)
{
   control->heuristic_parms = *parms;

   if(OutputLevel ||
#ifndef NDEBUG
false // true
#else
false
#endif
   )
   {
      fprintf(GlobalOut, "Selected heuristic:\n");
      HeuristicParmsPrint(stderr, &(control->heuristic_parms));
   }
   if(parms->mem_limit>2 && (parms->delete_bad_limit ==
                             DEFAULT_DELETE_BAD_LIMIT))
   {
      control->heuristic_parms.delete_bad_limit =
         (float)(parms->mem_limit-2)*0.7;
   }
   if(SpecNoEq(spec))
   {
      control->heuristic_parms.ac_handling = NoACHandling;
      OUTPRINT(1, COMCHAR" No equality, disabling AC handling.\n#\n");
   }
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/



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

   //printf("GetHeuristic(%s, ...)\n", source);

   /* StreamTypeOptionString is a hack here...it will give wrong error
      messages for internal strings (but the function will not be
      called with internal strings anyways...) */
   in=CreateScanner(StreamTypeOptionString, source, true, NULL, true);
   if(TestInpTok(in, OpenBracket))
   {
      HeuristicDefParse(control->hcbs, in, control->wfcbs,
                        control->ocb, state);
      CheckInpTok(in, NoToken); /* Make sure there is no trailing
                                   material - I've been bitten by an
                                   extra ')' cutting off a heuristic
                                   early. */
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
      /* res = HCBCreate(name, state, control, parms); */
      /* assert(res); */
      Error("Heuristic \'%s\' unknown\n", USAGE_ERROR, name);
   }
   FREE(name);
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
