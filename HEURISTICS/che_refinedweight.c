/*-----------------------------------------------------------------------

File  : che_refinedweight.c

Author: Stephan Schulz

Contents

  Functions realising clause evaluation with refined clause weights.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Wed Jun 17 00:11:03 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_refinedweight.h"


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
// Function: ClauseRefinedWeightInit()
//
//   Return an initialized WFCB for ClauseRefinedWeight evaluation.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p ClauseRefinedWeightInit(ClausePrioFun prio_fun, int fweight,
                int vweight, OCB_p ocb, double
                max_term_multiplier, double
                max_literal_multiplier, double
                pos_multiplier, double app_var_mult)
{
   RefinedWeightParam_p data = RefinedWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(ClauseRefinedWeightCompute, prio_fun,
          ClauseRefinedWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRefinedWeightParse()
//
//   Parse a refined clauseweight-definition.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p ClauseRefinedWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   
   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);
   
   AcceptInpTok(in, CloseBracket);

   return ClauseRefinedWeightInit(prio_fun, fweight, vweight, ocb,
              max_term_multiplier,
              max_literal_multiplier,
              pos_multiplier,
              app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: ClauseRefinedWeightCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseRefinedWeightCompute(void* data, Clause_p clause)
{
   RefinedWeightParam_p local = data;
   double res;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res =  ClauseWeight(clause,
             local->max_term_multiplier,
             local->max_literal_multiplier,
             local->pos_multiplier,
             local->vweight,
             local->fweight,
             local->app_var_mult,
             false);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRefinedWeight2Parse()
//
//   Refined weight counting EQ-Encoding
//
// Global Variables: -
//
// Side Effects    : Memory operations, reads input.
//
/----------------------------------------------------------------------*/

WFCB_p ClauseRefinedWeight2Parse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   WFCB_p tmp;

   tmp = ClauseRefinedWeightParse(in, ocb, state);
   tmp->wfcb_eval = ClauseRefinedWeight2Compute;

   return tmp;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseRefinedWeight2Compute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseRefinedWeight2Compute(void* data, Clause_p clause)
{
   RefinedWeightParam_p local = data;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   return ClauseWeight(clause,
             local->max_term_multiplier,
             local->max_literal_multiplier,
             local->pos_multiplier,
             local->vweight,
             local->fweight,
             local->app_var_mult,
             true);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseRefinedWeightExit()
//
//   Free the data entry in a clauseweight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseRefinedWeightExit(void* data)
{
   RefinedWeightParamCellFree(data);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


