/*-----------------------------------------------------------------------

  File  : che_dagweight.c

  Author: Stephan Schulz

  Contents

  Functions realising clause evaluation with clause weights.

  Copyright 2021 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Sun Apr 11 11:33:03 CEST 2021

-----------------------------------------------------------------------*/

#include "che_dagweight.h"


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
// Function: DAGWeightInit()
//
//   Return an initialized WFCB for DAGWeight evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p DAGWeightInit(ClausePrioFun prio_fun, int fweight, int
                     vweight, double pos_multiplier, long dup_weight,
                     bool pos_use_dag, bool pos_term_reset,
                     bool pos_eqn_reset, bool neg_use_dag,
                     bool neg_term_reset, bool neg_eqn_reset,
                     bool pos_neg_reset)
{
   DAGWeightParam_p data = DAGWeightParamCellAlloc();

   data->fweight        = fweight;
   data->vweight        = vweight;
   data->pos_multiplier = pos_multiplier;
   data->dup_weight     = dup_weight;
   data->pos_use_dag    = pos_use_dag;
   data->pos_term_reset = pos_term_reset;
   data->pos_eqn_reset  = pos_eqn_reset;
   data->neg_use_dag    = neg_use_dag;
   data->neg_term_reset = neg_term_reset;
   data->neg_eqn_reset  = neg_eqn_reset;
   data->pos_neg_reset  = pos_neg_reset;

   return WFCBAlloc(DAGWeightCompute, prio_fun,
          DAGWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: DAGWeightParse()
//
//   Parse a DAGWeight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p DAGWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier;
   long dup_weight;
   bool pos_use_dag, pos_term_reset, pos_eqn_reset,
      neg_use_dag, neg_term_reset, neg_eqn_reset, pos_neg_reset;


   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   dup_weight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_use_dag = ParseBool(in);
   AcceptInpTok(in, Comma);
   pos_term_reset = ParseBool(in);
   AcceptInpTok(in, Comma);
   pos_eqn_reset = ParseBool(in);
   AcceptInpTok(in, Comma);
   neg_use_dag = ParseBool(in);
   AcceptInpTok(in, Comma);
   neg_term_reset = ParseBool(in);
   AcceptInpTok(in, Comma);
   neg_eqn_reset = ParseBool(in);
   AcceptInpTok(in, Comma);
   pos_neg_reset = ParseBool(in);
   AcceptInpTok(in, CloseBracket);

   return DAGWeightInit(prio_fun, fweight, vweight, pos_multiplier,
                        dup_weight, pos_use_dag, pos_term_reset,
                        pos_eqn_reset, neg_use_dag, neg_term_reset,
                        neg_eqn_reset, pos_neg_reset);
}


/*-----------------------------------------------------------------------
//
// Function: DAGWeightCompute()
//
//   Compute a dag-evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double DAGWeightCompute(void* data, Clause_p clause)
{
   DAGWeightParam_p local = data;
   double res = 0;
   Eqn_p handle;

   EqnListTermDelProp(clause->literals, TPOpFlag);

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         if(local->pos_use_dag)
         {
            res +=  local->pos_multiplier *
               EqnDAGWeight(handle, 1, local->vweight, local->fweight,
                            local->dup_weight, local->pos_eqn_reset,
                            local->pos_term_reset);
         }
         else
         {
            res += LiteralWeight(handle, 1, 1,  local->pos_multiplier,
                                 local->vweight, local->fweight, 1, false);
         }
      }
   }

   if(local->pos_neg_reset)
   {
      EqnListSignedTermDelProp(clause->literals, TPOpFlag, false, true);
   }
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsNegative(handle))
      {
         if(local->neg_use_dag)
         {
            res +=  EqnDAGWeight(handle, 1, local->vweight, local->fweight,
                                 local->dup_weight, local->neg_eqn_reset,
                                 local->neg_term_reset);
         }
         else
         {
            res += LiteralWeight(handle, 1, 1,  1,
                                 local->vweight, local->fweight, 1, false);
         }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: DAGWeightExit()
//
//   Free the data entry in a DAGWeight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void DAGWeightExit(void* data)
{
   DAGWeightParamCellFree(data);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
