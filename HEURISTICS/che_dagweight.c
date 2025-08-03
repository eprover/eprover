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
               EqnDAGWeight(handle, 1, 1, local->vweight, local->fweight,
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
            res +=  EqnDAGWeight(handle, 1, 1, local->vweight, local->fweight,
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
   // printf("\n"COMCHAR" Eval: %6.2f: ", res);
   // ClauseTSTPPrint(stdout, clause, true,true);
   // printf("\n");
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



/*-----------------------------------------------------------------------
//
// Function: RDAGWeightInit()
//
//   Return an initialized WFCB for RDAGWeightCompute().
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p RDAGWeightInit(ClausePrioFun prio_fun,
                      OCB_p ocb,
                      long fweight,
                      long vweight,
                      long dup_weight,
                      double uniqmax_term_multiplier,
                      double max_term_multiplier,
                      double max_literal_multiplier,
                      double pos_multiplier)
{
   RDAGWeightParam_p data = RDAGWeightParamCellAlloc();

   data->ocb                    = ocb;
   data->fweight                = fweight;
   data->vweight                = vweight;
   data->dup_weight             = dup_weight;
   data->uniqmax_term_multiplier= uniqmax_term_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->pos_multiplier         = pos_multiplier;

   return WFCBAlloc(RDAGWeightCompute, prio_fun,
                    RDAGWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: RDAGWeightParse()
//
//   Parse a refined DAG-clauseweight-definition.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p RDAGWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   long fweight, vweight, dup_weight;
   double pos_multiplier, max_term_multiplier,
      uniqmax_term_multiplier, max_literal_multiplier;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   dup_weight = ParseInt(in);
   AcceptInpTok(in, Comma);
   uniqmax_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   AcceptInpTok(in, CloseBracket);

   return RDAGWeightInit(prio_fun,
                         ocb,
                         fweight, vweight, dup_weight,
                         uniqmax_term_multiplier,
                         max_term_multiplier,
                         max_literal_multiplier,
                         pos_multiplier);
}



/*-----------------------------------------------------------------------
//
// Function: RDAGWeightCompute()
//
//   Compute a refined-dag-evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double RDAGWeightCompute(void* data, Clause_p clause)
{
   RDAGWeightParam_p local = data;
   double res = 0, pos_multiplier;
   Eqn_p handle;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   EqnListTermDelProp(clause->literals, TPOpFlag);

   for(handle = clause->literals; handle; handle = handle->next)
   {
      pos_multiplier = EqnIsPositive(handle)?local->pos_multiplier:1;
      //printf("pos_multiplier: %f", pos_multiplier);
      res += pos_multiplier *
         EqnDAGWeight(handle,
                      local->uniqmax_term_multiplier,
                      local->max_term_multiplier,
                      local->vweight,
                      local->fweight,
                      local->dup_weight,
                      true, true);
   }
   //printf("\n"COMCHAR" Eval: %6.2f: ", res);
   //ClauseTSTPPrint(stdout, clause, true,true);
   //printf("\n");
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: RDAGWeightExit()
//
//   Free the data entry in a RDAGWeight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void RDAGWeightExit(void* data)
{
   RDAGWeightParamCellFree(data);
}



/*-----------------------------------------------------------------------
//
// Function: RDAGWeight2Init()
//
//   Return an initialized WFCB for RDAGWeight2Compute().
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p RDAGWeight2Init(ClausePrioFun prio_fun,
                      OCB_p ocb,
                      long fweight,
                      long vweight,
                      long dup_weight,
                      double max_term_multiplier,
                      double pos_multiplier)
{
   RDAGWeightParam_p data = RDAGWeightParamCellAlloc();

   data->ocb                    = ocb;
   data->fweight                = fweight;
   data->vweight                = vweight;
   data->dup_weight             = dup_weight;
   data->uniqmax_term_multiplier= 1; /* Unused */
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = 1; /* unused */
   data->pos_multiplier         = pos_multiplier;

   /* RDAGWeightExit intentionally reused  */
   return WFCBAlloc(RDAGWeight2Compute, prio_fun,
                    RDAGWeightExit, data);

}


/*-----------------------------------------------------------------------
//
// Function: RDAGWeight2Parse()
//
//   Parse a refined Twee-style DAG2-clauseweight-definition.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p RDAGWeight2Parse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   long fweight, vweight, dup_weight;
   double pos_multiplier, max_term_multiplier;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   dup_weight = ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   AcceptInpTok(in, CloseBracket);

   return RDAGWeight2Init(prio_fun,
                         ocb,
                         fweight, vweight, dup_weight,
                         max_term_multiplier,
                         pos_multiplier);
}



/*-----------------------------------------------------------------------
//
// Function: RDAGWeight2Compute()
//
//   Compute a Twee-style dag-evaluation for a clause. The "larger"
//   (by wighted symbol count) is given higher weigth. Term orderings
//   are ignored.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double RDAGWeight2Compute(void* data, Clause_p clause)
{
   RDAGWeightParam_p local = data;
   double res = 0, pos_multiplier;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      pos_multiplier = EqnIsPositive(handle)?local->pos_multiplier:1;
      //printf("pos_multiplier: %f", pos_multiplier);
      res += pos_multiplier *
         EqnDAGWeight2(handle,
                      local->max_term_multiplier,
                      local->vweight,
                      local->fweight,
                      local->dup_weight);
   }
   //printf("\n"COMCHAR" Eval: %6.2f: ", res);
   //ClauseTSTPPrint(stdout, clause, true,true);
   //printf("\n");
   return res;
}




/*-----------------------------------------------------------------------
//
// Function: RDAGWeight3Init()
//
//   Return an initialized WFCB for RDAGWeight3Compute().
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p RDAGWeight3Init(ClausePrioFun prio_fun,
                       OCB_p ocb,
                       long fweight,
                       long vweight,
                       long nfweight,
                       long nvweight,
                       long dup_weight,
                       double max_term_multiplier,
                       double pos_multiplier,
                       double pneq_multiplier,
                       double nneq_multiplier)
{
   RDAGWeightParam_p data = RDAGWeightParamCellAlloc();

   data->ocb                    = ocb;
   data->fweight                = fweight;
   data->vweight                = vweight;
   data->nfweight               = nfweight;
   data->nvweight               = nvweight;
   data->dup_weight             = dup_weight;
   data->uniqmax_term_multiplier= 1; /* Unused */
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = 1; /* unused */
   data->pos_multiplier         = pos_multiplier;
   data->pneq_multiplier        = pneq_multiplier;
   data->nneq_multiplier        = nneq_multiplier;

   /* RDAGWeightExit intentionally reused  */
   return WFCBAlloc(RDAGWeight3Compute, prio_fun,
                    RDAGWeightExit, data);

}


/*-----------------------------------------------------------------------
//
// Function: RDAGWeight3Parse()
//
//   Parse a refined Twee-style DAG2-clauseweight-definition.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p RDAGWeight3Parse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   long fweight, vweight, nfweight, nvweight, dup_weight;
   double pos_multiplier, max_term_multiplier, pneq_multiplier, nneq_multiplier;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   nfweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   nvweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   dup_weight = ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pneq_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   nneq_multiplier = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);

   return RDAGWeight3Init(prio_fun,
                          ocb,
                          fweight, vweight,
                          nfweight, nvweight,
                          dup_weight,
                          max_term_multiplier,
                          pos_multiplier,
                          pneq_multiplier,
                          nneq_multiplier);
}



/*-----------------------------------------------------------------------
//
// Function: RDAGWeight3Compute()
//
//   Compute a mixed dag weight. For positive literals, both terms are
//   independendly computed, either with normal weights, or with dag
//   weights. For negative literals, we use one dag.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double RDAGWeight3Compute(void* data, Clause_p clause)
{
   RDAGWeightParam_p local = data;
   double res = 0, litw, lweight, rweight;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         lweight = TermDAGWeight(handle->lterm, local->fweight,
                                 local->vweight, local->dup_weight, true);
         rweight = TermDAGWeight(handle->rterm, local->fweight,
                                 local->vweight, local->dup_weight, true);
         if(lweight >= rweight)
         {
            lweight *= local->max_term_multiplier;
         }
         else
         {
            rweight *= local->max_term_multiplier;
         }
         litw = (lweight+rweight)*local->pos_multiplier;
         if(EqnIsEquLit(handle))
         {
            litw *= local->pneq_multiplier;
         }
      }
      else
      {
         EqnTermDelProp(handle, TPOpFlag);
         lweight = TermDAGWeight(handle->lterm, local->nfweight,
                                 local->nvweight, local->dup_weight, false);
         rweight = TermDAGWeight(handle->rterm, local->nfweight,
                                 local->nvweight, local->dup_weight, false);
         litw = lweight+rweight;
         if(EqnIsEquLit(handle))
         {
            litw *= local->nneq_multiplier;
         }
      }
      res = res+litw;
   }
   return res;
}




/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
