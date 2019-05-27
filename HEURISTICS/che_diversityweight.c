/*-----------------------------------------------------------------------

  File  : che_diversityweight.c

  Author: Stephan Schulz

  Contents

  Functions for the evaluation of a clause by refined diversity clause
  weight, using weight penalty factors for maximal terms and literals,
  and penalties for clauses with many different function symbols and
  variables.

  Copyright 2019 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Mon May 27 01:01:22 CEST 2019

-----------------------------------------------------------------------*/

#include "che_diversityweight.h"


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
// Function: DiversityWeightInit()
//
//   Return an initialized WFCB for DiversityWeight evaluation.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p DiversityWeightInit(ClausePrioFun prio_fun, int fweight,
                           int vweight, OCB_p ocb,
                           double max_term_multiplier,
                           double max_literal_multiplier,
                           double pos_multiplier,
                           double fdiff1weight,
                           double fdiff2weight,
                           double vdiff1weight,
                           double vdiff2weight,
                           double app_var_mult)
{
   DiversityWeightParam_p data = DiversityWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->ocb                    = ocb;
   data->fdiff1weight           = fdiff1weight;
   data->fdiff2weight           = fdiff2weight;
   data->vdiff1weight           = vdiff1weight;
   data->vdiff2weight           = vdiff2weight;

   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(DiversityWeightCompute, prio_fun,
          DiversityWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: DiversityWeightParse()
//
//   Parse a DiversityWeight-definition.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p DiversityWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier,
      fdiff1weight, fdiff2weight,
      vdiff1weight, vdiff2weight, app_var_mult = APP_VAR_MULT_DEFAULT;

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

   AcceptInpTok(in, Comma);
   fdiff1weight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   fdiff2weight = ParseFloat(in);

   AcceptInpTok(in, Comma);
   vdiff1weight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   vdiff2weight = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return DiversityWeightInit(prio_fun, fweight, vweight, ocb,
                              max_term_multiplier,
                              max_literal_multiplier,
                              pos_multiplier,
                              fdiff1weight,
                              fdiff2weight,
                              vdiff1weight,
                              vdiff2weight,
                              app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: DiversityWeightCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double DiversityWeightCompute(void* data, Clause_p clause)
{
   DiversityWeightParam_p local = data;
   double res;
   PStack_p stack;
   PTree_p tree;
   long f_diversity, v_diversity;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res =  ClauseWeight(clause,
             local->max_term_multiplier,
             local->max_literal_multiplier,
             local->pos_multiplier,
             local->vweight,
             local->fweight,
             local->app_var_mult,
             false);

   stack = PStackAlloc();
   f_diversity = ClauseReturnFCodes(clause, stack);
   PStackFree(stack);

   tree = NULL;
   v_diversity = ClauseCollectVariables(clause, &tree);
   PTreeFree(tree);

   res += f_diversity*(local->fdiff2weight*f_diversity+local->fdiff1weight);
   res += v_diversity*(local->vdiff2weight*v_diversity+local->vdiff1weight);

   return res;
}



/*-----------------------------------------------------------------------
//
// Function: DiversityWeightExit()
//
//   Free the data entry in a clauseweight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DiversityWeightExit(void* data)
{
   DiversityWeightParamCellFree(data);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
