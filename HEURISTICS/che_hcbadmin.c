/*-----------------------------------------------------------------------

  File  : che_hcbadmin.c

  Author: Stephan Schulz

  Contents

  Functions for administrating sets of predefined HCBs.

  Copyright 1998-2020 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Wed Dec  9 02:44:26 MET 1998

  -----------------------------------------------------------------------*/

#include "che_hcbadmin.h"



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
// Function: parse_single_wfcb_item()
//
//   Parse a single wfcb-item "name(steps)" and insert it into the
//   HCB.
//
// Global Variables: -
//
// Side Effects    : Input, changes HCB (of course)
//
/----------------------------------------------------------------------*/

static void parse_single_wfcb_item(HCB_p hcb, Scanner_p in,
                                   WFCBAdmin_p wfcbs, OCB_p ocb,
                                   ProofState_p state)
{
   WFCB_p wfcb;
   long   steps;
   char   *name;

   steps = AktToken(in)->numval;
   if(steps<=0)
   {
      AktTokenError(in,
                    "Value >0 expected in heuristic evaluation"
                    " function description",
                    false);
   }
   AcceptInpTok(in, PosInt);
   AcceptInpTok(in, Mult|Fullstop);

   CheckInpTok(in, Identifier);

   if(TestTok(LookToken(in,1), OpenBracket|EqualSign))
   {
      name = WeightFunDefParse(wfcbs, in, ocb, state);
      wfcb = WFCBAdminFindWFCB(wfcbs, name);
   }
   else
   {
      name = DStrView(AktToken(in)->literal);
      wfcb = WFCBAdminFindWFCB(wfcbs, name);
      NextToken(in);
   }
   if(!wfcb)
   {
      AktTokenError(in, "Not a valid evaluation function specifier",
                    false);
   }
   HCBAddWFCB(hcb, wfcb, steps);
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: HCBAdminAlloc()
//
//   Allocate an empty initialized HCBAdminCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

HCBAdmin_p HCBAdminAlloc(void)
{
   HCBAdmin_p handle = HCBAdminCellAlloc();

   handle->names   = PStackAlloc();
   handle->hcb_set = PStackAlloc();

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: HCBAdminFree()
//
//   Free a HCBAdminCell. Will also free stored hcb's and names.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void HCBAdminFree(HCBAdmin_p junk)
{
   char* name;

   assert(junk);
   assert(junk->names);
   assert(junk->hcb_set);

   while(!PStackEmpty(junk->names))
   {
      name = PStackPopP(junk->names);
      FREE(name);
   }
   PStackFree(junk->names);

   while(!PStackEmpty(junk->hcb_set))
   {
      HCBFree(PStackPopP(junk->hcb_set));
   }
   PStackFree(junk->hcb_set);

   HCBAdminCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: HCBAdminAddHCB()
//
//   Add a HCB under a given name to the HCB-Set. Return index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long HCBAdminAddHCB(HCBAdmin_p set, char* name, HCB_p hcb)
{
   assert(set);
   assert(set->names);
   assert(set->hcb_set);
   assert(name);
   assert(hcb);

   VERBOSE2(fprintf(stderr, "%s: Adding heuristic '%s'.\n",
                    ProgName, name););
   PStackPushP(set->names, SecureStrdup(name));
   PStackPushP(set->hcb_set, hcb);

   return PStackGetSP(set->hcb_set)-1;
}


/*-----------------------------------------------------------------------
//
// Function: HCBAdminFindHCB()
//
//   Given a name and a HCB-Set, return the matching HCB (or
//   NULL). Always returns the last HCB with the same name, so you
//   can redefine predefined heuristics!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

HCB_p HCBAdminFindHCB(HCBAdmin_p set, char* name)
{
   PStackPointer i;
   HCB_p res = NULL;

   assert(set);
   assert(set->names);
   assert(set->hcb_set);
   assert(name);


   for(i = PStackGetSP(set->names)-1; i>=0; i--)
   {
      if(strcmp(name, PStackElementP(set->names, i))==0)
      {
         res = PStackElementP(set->hcb_set, i);
         break;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicParse()
//
//   Parse a heuristic.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

HCB_p HeuristicParse(Scanner_p in, WFCBAdmin_p wfcbs, OCB_p ocb,
                     ProofState_p state)
{
   HCB_p  hcb = HCBAlloc();

   AcceptInpTok(in, OpenBracket);

   parse_single_wfcb_item(hcb, in, wfcbs, ocb, state);

   while(TestInpTok(in, Comma))
   {
      AcceptInpTok(in, Comma);
      parse_single_wfcb_item(hcb, in, wfcbs, ocb, state);
   }
   AcceptInpTok(in, CloseBracket);
   return hcb;
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicDefParse()
//
//   Parse a heuristics definition and add it to the set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long HeuristicDefParse(HCBAdmin_p set, Scanner_p in, WFCBAdmin_p
                       wfcbs, OCB_p ocb, ProofState_p state)
{
   char* name;
   HCB_p hcb;
   long   res;

   if(TestInpTok(in, OpenBracket))
   {
      name = SecureStrdup("Default");
   }
   else
   {
      CheckInpTok(in, Identifier);
      name = SecureStrdup(DStrView(AktToken(in)->literal));
      /* All this strdup'ing is inefficient, but uncritical */
      NextToken(in);
      AcceptInpTok(in, EqualSign);
   }
   hcb = HeuristicParse(in, wfcbs, ocb, state);
   res = HCBAdminAddHCB(set, name, hcb);
   FREE(name);

   return res;
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicDefListParse()
//
//   Parse a list of heuristics_definitions definitions.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long HeuristicDefListParse(HCBAdmin_p set, Scanner_p in, WFCBAdmin_p
                           wfcbs, OCB_p ocb, ProofState_p state)
{
   long res = PStackGetSP(set->names);

   while((TestInpTok(in, Identifier)&&TestTok(LookToken(in,1), EqualSign))
         ||TestInpTok(in, OpenBracket))
   {
      res = HeuristicDefParse(set, in, wfcbs, ocb, state);
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
