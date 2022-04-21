/*-----------------------------------------------------------------------

File  : che_wfcbadmin.c

Author: Stephan Schulz

Contents

  Functions for administrating sets of predefined WFCBs.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Dec  9 02:44:26 MET 1998
    New

-----------------------------------------------------------------------*/

#include "che_wfcbadmin.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

char* WeightFunParseFunNames[]=
{
   "Clauseweight",
   "ClauseLMaxWeight",
   "ClauseCMaxWeight",
   "Uniqweight",
   "Defaultweight",
   "DAGweight",
   "RDAGweight",
   "RDAGweight2",
   "RDAGweight3",
   "Refinedweight",
   "Refinedweight2",
   "Diversityweight",
   "PNRefinedweight",
   "TPTPTypeweight",
   "Sigweight",
   "NLweight",
   "RandomWeight",
   "SymbolTypeweight",
   "Depthweight",
   "WLessDWeight",
   "Proofweight",
   "Orientweight",
   "OrientLMaxWeight",
   "Simweight",
   "FIFOWeight",
   "LIFOWeight",
   "StaggeredWeight",
   "ClauseWeightAge",
   "TSMWeight",
   "TSMRWeight",
   "ConjectureSymbolWeight",
   "ConjectureGeneralSymbolWeight",
   "ConjectureRelativeSymbolWeight",
   "ConjectureRelativeTypeSymbolWeight",
   "ConjectureTypeBasedWeight",
   "RelevanceLevelWeight",
   "RelevanceLevelWeight2",
   "FunWeight",
   "SymOffsetWeight",
   "ConjectureRelativeTermWeight",
   "ConjectureTermTfIdfWeight",
   "ConjectureLevDistanceWeight",
   "ConjectureTreeDistanceWeight",
   "ConjectureTermPrefixWeight",
   "ConjectureStrucDistanceWeight",
   NULL
};

static WeightFunParseFun parse_fun_array[]=
{
   ClauseWeightParse,
   LMaxWeightParse,
   CMaxWeightParse,
   UniqWeightParse, /* NOTE2ME:  did not change it */
   DefaultWeightParse, /* NOTE2ME:  did not change it */
   DAGWeightParse,
   RDAGWeightParse,
   RDAGWeight2Parse,
   RDAGWeight3Parse,
   ClauseRefinedWeightParse,
   ClauseRefinedWeight2Parse,
   DiversityWeightParse,
   PNRefinedWeightParse,
   TPTPTypeWeightParse,
   SigWeightParse,
   NLWeightParse,
   RandWeightParse,
   SymTypeWeightParse,
   DepthWeightParse,
   WeightLessDepthParse,
   ProofWeightParse,
   ClauseOrientWeightParse,
   OrientLMaxWeightParse,
   SimWeightParse,
   FIFOEvalParse,  /* NOTE2ME:  did not change it */
   LIFOEvalParse, /* NOTE2ME:  did not change it */
   StaggeredWeightParse, /* NOTE2ME:  did not change it */
   ClauseWeightAgeParse,
   TSMWeightParse /* NOTE2ME:  did not change it */,
   TSMRWeightParse /* NOTE2ME:  did not change it */,
   ConjectureSimplifiedSymbolWeightParse,
   ConjectureSymbolWeightParse,
   ConjectureRelativeSymbolWeightParse,
   ConjectureRelativeSymbolTypeWeightParse,
   ConjectureTypeBasedWeightParse,
   RelevanceLevelWeightParse,
   RelevanceLevelWeightParse2,
   FunWeightParse,
   SymOffsetWeightParse,
   ConjectureRelativeTermWeightParse,
   ConjectureTermTfIdfWeightParse,
   ConjectureLevDistanceWeightParse,
   ConjectureTreeDistanceWeightParse,
   ConjectureTermPrefixWeightParse,
   ConjectureStrucDistanceWeightParse,
   (WeightFunParseFun)NULL
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

/*-----------------------------------------------------------------------
//
// Function: WFCBAdminAlloc()
//
//   Allocate an empty initialized WFCBAdminCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCBAdmin_p WFCBAdminAlloc(void)
{
   WFCBAdmin_p handle = WFCBAdminCellAlloc();

   handle->names        = PStackAlloc();
   handle->wfcb_set     = PStackAlloc();
   handle->anon_counter = 0;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: WFCBAdminFree()
//
//   Free a WFCBAdminCell. Will also free stored wfcb's and names.
//
// Global Variables: -
//
// Side Effects    : Memory operations.
//
/----------------------------------------------------------------------*/

void WFCBAdminFree(WFCBAdmin_p junk)
{
   char* name;

   assert(junk);
   assert(junk->names);
   assert(junk->wfcb_set);

   while(!PStackEmpty(junk->names))
   {
      name = PStackPopP(junk->names);
      FREE(name);
   }
   PStackFree(junk->names);

   while(!PStackEmpty(junk->wfcb_set))
   {
      WFCBFree(PStackPopP(junk->wfcb_set));
   }
   PStackFree(junk->wfcb_set);

   WFCBAdminCellFree(junk);
}


/*-----------------------------------------------------------------------
//
// Function: WFCBAdminAddWFCB()
//
//   Add a WFCB under a given name to the WFCB-Set. Return index.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long WFCBAdminAddWFCB(WFCBAdmin_p set, char* name, WFCB_p wfcb)
{
   assert(set);
   assert(set->names);
   assert(set->wfcb_set);
   assert(name);
   assert(wfcb);

   VERBOSE2(fprintf(stderr, "%s: Adding weight function '%s'.\n",
         ProgName, name););
   PStackPushP(set->names, SecureStrdup(name));
   PStackPushP(set->wfcb_set, wfcb);

   return PStackGetSP(set->wfcb_set)-1;
}


/*-----------------------------------------------------------------------
//
// Function: WFCBAdminFindWFCB()
//
//   Given a name and a WFCB-Set, return the matching WFCB (or
//   NULL). Always returns the last WFCB with the same name, so you
//   can redefine predefined weight functions!
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p WFCBAdminFindWFCB(WFCBAdmin_p set, char* name)
{
   PStackPointer i;
   WFCB_p res = NULL;

   assert(set);
   assert(set->names);
   assert(set->wfcb_set);
   assert(name);


   for(i = PStackGetSP(set->names)-1; i>=0; i--)
   {
      if(strcmp(name, PStackElementP(set->names, i))==0)
      {
         res = PStackElementP(set->wfcb_set, i);
         break;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: GetWeightFunParseFun()
//
//   Given a name of a weight function, return a parse function for
//   it.
//
// Global Variables: WeightFunParseFunNames
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WeightFunParseFun GetWeightFunParseFun(char* name)
{
   int index;

   assert(name);

   index = StringIndex(name, WeightFunParseFunNames);

   if(index>=0)
   {
      return parse_fun_array[index];
   }
   return (WeightFunParseFun)NULL;
}


/*-----------------------------------------------------------------------
//
// Function: WeightFunParse()
//
//   Parse a weight function.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p WeightFunParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   WeightFunParseFun parse_fun;

   CheckInpTok(in, Identifier);
   parse_fun = GetWeightFunParseFun(DStrView(AktToken(in)->literal));

   if(!parse_fun)
   {
      AktTokenError(in, "Not a valid weight function specifier",
          false);
   }
   NextToken(in);
   assert(parse_fun);
   return parse_fun(in, ocb, state);
}


/*-----------------------------------------------------------------------
//
// Function: WeightFunDefParse()
//
//   Parse a weight function definition and add it to the set. Returns
//   a pointer to the name.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

char* WeightFunDefParse(WFCBAdmin_p set, Scanner_p in, OCB_p ocb,
             ProofState_p state)
{
   char* name = NULL;
   WFCB_p wfcb;
   long   res;
   char   anon_name[14];

   if(TestTok(LookToken(in,1), EqualSign))
   {
      CheckInpTok(in, Identifier);
      name = SecureStrdup(DStrView(AktToken(in)->literal));
      /* All this strdup'ing is inefficient, but uncritical */
      NextToken(in);
      AcceptInpTok(in, EqualSign);
   }
   else
   {
      sprintf(anon_name, "~$%09ld", set->anon_counter++);
      name = anon_name;
   }
   wfcb = WeightFunParse(in, ocb, state);
   res = WFCBAdminAddWFCB(set, name, wfcb);
   if(name!=anon_name)
   {
      FREE(name);
   }
   return PStackElementP(set->names,res);
}


/*-----------------------------------------------------------------------
//
// Function: WeightFunDefListParse()
//
//   Parse a list of weight function definitions. Return number of
//   entries parsed.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

long WeightFunDefListParse(WFCBAdmin_p set, Scanner_p in, OCB_p ocb,
            ProofState_p state)
{
   long res = 0;

   while(TestInpTok(in, Identifier)&&
    TestTok(LookToken(in,1), EqualSign|OpenBracket))
   {
      WeightFunDefParse(set, in, ocb, state);
      res++;
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
