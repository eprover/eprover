/*-----------------------------------------------------------------------

File  : che_varweights.c

Author: Stephan Schulz

Contents

  New weight functions.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Jan 28 17:45:23 MET 1999
    New

-----------------------------------------------------------------------*/

#include "che_varweights.h"


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
// Function: TPTPTypeWeightInit()
//
//   Initialize a WFCB for a TPTPTypeWeight-Evaluation function, that
//   modifies a base refinedweight according to clause type.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p TPTPTypeWeightInit(ClausePrioFun prio_fun, int fweight,
           int vweight, OCB_p ocb, double
           max_term_multiplier, double
           max_literal_multiplier, double
           pos_multiplier, double
           conjecture_multiplier, double
           hypothesis_multiplier, double
           app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->conjecture_multiplier  = conjecture_multiplier;
   data->hypothesis_multiplier  = hypothesis_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(TPTPTypeWeightCompute, prio_fun,
                    VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: TPTPTypeWeightParse()
//
//   Parse a TPTPTypeWeight-Evaluation function.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory management
//
/----------------------------------------------------------------------*/

WFCB_p TPTPTypeWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, conjecture_multiplier,
                 hypothesis_multiplier, app_var_mult = APP_VAR_MULT_DEFAULT;

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
   conjecture_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   hypothesis_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return TPTPTypeWeightInit(prio_fun, fweight, vweight, ocb,
              max_term_multiplier,
              max_literal_multiplier, pos_multiplier,
              conjecture_multiplier,
              hypothesis_multiplier, app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: TPTPTypeWeightCompute()
//
//   Compute a weight and adjust it for clause type.
//
// Global Variables: -
//
// Side Effects    : May orient clause
//
/----------------------------------------------------------------------*/

double TPTPTypeWeightCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseWeight(clause,
               local->max_term_multiplier,
               local->max_literal_multiplier,
               local->pos_multiplier,
               local->vweight,
               local->fweight,
               local->app_var_mult,
               false);
   switch(ClauseQueryTPTPType(clause))
   {
   case CPTypeHypothesis:
    res = res * local->hypothesis_multiplier;
    break;
   case CPTypeConjecture:
    res = res * local->conjecture_multiplier;
    break;
   default:
    break;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: SigWeightInit()
//
//   Initialize a WFCB for a SigWeight-Evaluation function, that
//   modifies a base refinedweight according to number of different
//   function symbols occuring in the clause
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p SigWeightInit(ClausePrioFun prio_fun, int fweight,
           int vweight, OCB_p ocb, double
           max_term_multiplier, double
           max_literal_multiplier, double
           pos_multiplier, double sig_size_multiplier,
           double app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->sig_size_multiplier    = sig_size_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(SigWeightCompute, prio_fun,
                    VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: SigWeightParse()
//
//   Parse a SigWeight-Evaluation function.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory management
//
/----------------------------------------------------------------------*/

WFCB_p SigWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, sig_size_multiplier,
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
   AcceptInpTok(in, Comma);
   sig_size_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return SigWeightInit(prio_fun, fweight, vweight, ocb,
         max_term_multiplier, max_literal_multiplier,
         pos_multiplier, sig_size_multiplier, app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: SigWeightCompute()
//
//   Compute a weight and adjust it for clause type.
//
// Global Variables: -
//
// Side Effects    : May orient clause
//
/----------------------------------------------------------------------*/

double SigWeightCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res;
   double           modify;
   long             sig_size;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseWeight(clause,
            local->max_term_multiplier,
            local->max_literal_multiplier,
            local->pos_multiplier,
            local->vweight,
            local->fweight,
            local->app_var_mult,
            false);
   sig_size = ClauseCountExtSymbols(clause, local->ocb->sig, 0);
   modify = (sig_size*local->sig_size_multiplier/
        (MAX(1,SigExternalSymbols(local->ocb->sig))));
   /* printf("sig_size = %ld of %ld, sig_size_multiplier = %f, modify = %f\n",
     sig_size, SigExternalSymbols(local->ocb->sig),
     local->sig_size_multiplier, modify); */
   res = res * (1+modify);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ProofWeightInit()
//
//   Initialize a WFCB for a ProofWeight-Evaluation function, that
//   modifies a base refinedweight according to clause proof lenght
//   and depth.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p ProofWeightInit(ClausePrioFun prio_fun, int fweight,
           int vweight, OCB_p ocb, double
           max_term_multiplier, double
           max_literal_multiplier, double
           pos_multiplier, double
           proof_size_multiplier, double
           proof_depth_multiplier, double
           app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->proof_size_multiplier  = proof_size_multiplier;
   data->proof_depth_multiplier = proof_depth_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(ProofWeightCompute, prio_fun,
                    VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: ProofWeightParse()
//
//   Parse a ProofWeight-Evaluation function.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory management
//
/----------------------------------------------------------------------*/

WFCB_p ProofWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, proof_size_multiplier,
                 proof_depth_multiplier,
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
   AcceptInpTok(in, Comma);
   proof_size_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   proof_depth_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ProofWeightInit(prio_fun, fweight, vweight, ocb,
              max_term_multiplier,
              max_literal_multiplier, pos_multiplier,
              proof_size_multiplier,
              proof_depth_multiplier,
              app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: ProofWeightCompute()
//
//   Compute a weight and adjust it for proof depth and lenght.
//
// Global Variables: -
//
// Side Effects    : May orient clause
//
/----------------------------------------------------------------------*/

double ProofWeightCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseWeight(clause,
            local->max_term_multiplier,
            local->max_literal_multiplier,
            local->pos_multiplier,
            local->vweight,
            local->fweight,
            local->app_var_mult,
            false);
   res *= (1+local->proof_depth_multiplier*(1/(double)(clause->proof_depth+1)));
   res *= (1+local->proof_size_multiplier*(1/(double)(clause->proof_size+1)));
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: DepthWeightInit()
//
//   Initialize a WFCB for a DepthWeight-Evaluation function that
//   uses both dept and weight.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p DepthWeightInit(ClausePrioFun prio_fun, int fweight,
           int vweight, OCB_p ocb, double
           max_term_multiplier, double
           max_literal_multiplier, double
           pos_multiplier, double
             term_weight_multiplier, double
             app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->term_weight_multiplier = term_weight_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(DepthWeightCompute, prio_fun,
                    VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: DepthWeightParse()
//
//   Parse a DepthWeight-Evaluation function.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory management
//
/----------------------------------------------------------------------*/

WFCB_p DepthWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
         state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, term_weight_multiplier,
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
   AcceptInpTok(in, Comma);
   term_weight_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return DepthWeightInit(prio_fun, fweight, vweight, ocb,
           max_term_multiplier, max_literal_multiplier,
           pos_multiplier, term_weight_multiplier,
           app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: DepthWeightCompute()
//
//   Compute a weight and adjust it for clause type.
//
// Global Variables: -
//
// Side Effects    : May orient clause
//
/----------------------------------------------------------------------*/

double DepthWeightCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res = 0, lweight, rweight, weight;
   Eqn_p            handle;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   for(handle = clause->literals; handle; handle = handle->next)
   {
      lweight = TermDepth(handle->lterm)+
    local->term_weight_multiplier*TermWeight(handle->lterm,
                   local->vweight,
                   local->fweight);
      lweight = lweight*local->max_term_multiplier*(TermIsAppliedFreeVar(handle->lterm)?local->app_var_mult:1);
      rweight = TermDepth(handle->rterm)+
    local->term_weight_multiplier*TermWeight(handle->rterm,
                   local->vweight,
                   local->fweight)*(TermIsAppliedFreeVar(handle->rterm)?local->app_var_mult:1);
      if(!EqnIsOriented(handle))
      {
    rweight = rweight*local->max_term_multiplier;
      }
      weight = lweight+rweight;

      if(EqnIsPositive(handle))
      {
    weight = weight*local->pos_multiplier;
      }
      if(EqnIsMaximal(handle))
      {
    weight = weight*local->max_literal_multiplier;
      }
      res += weight;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: WeightLessDepthInit()
//
//   Initialize a function that evaluates terms as weight-gamma*dpth.
//
// Global Variables: -
//
// Side Effects    : As all these functions ;-)
//
/----------------------------------------------------------------------*/

WFCB_p WeightLessDepthInit(ClausePrioFun prio_fun, int fweight,
             int vweight, OCB_p ocb, double
             max_term_multiplier, double
             max_literal_multiplier, double
             pos_multiplier, double term_depth_multiplier,
             double app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->term_depth_multiplier  = term_depth_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(WeightLessDepthCompute, prio_fun,
                    VarWeightExit, data);

}

/*-----------------------------------------------------------------------
//
// Function: WeightLessDepthParse()
//
//   Parse the above function.
//
// Global Variables: -
//
// Side Effects    : As always.
//
/----------------------------------------------------------------------*/

WFCB_p WeightLessDepthParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, term_depth_multiplier,
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
   AcceptInpTok(in, Comma);
   term_depth_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return WeightLessDepthInit(prio_fun, fweight, vweight, ocb,
               max_term_multiplier, max_literal_multiplier,
               pos_multiplier, term_depth_multiplier, app_var_mult);
}

/*-----------------------------------------------------------------------
//
// Function: WeightLessDepthCompute()
//
//   Compute the evaluation function.
//
// Global Variables: -
//
// Side Effects    : May orient the clause.
//
/----------------------------------------------------------------------*/

double WeightLessDepthCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res = 0, lweight, rweight, weight;
   Eqn_p            handle;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   for(handle = clause->literals; handle; handle = handle->next)
   {
      lweight = TermWeight(handle->lterm, local->vweight, local->fweight)
    - local->term_depth_multiplier*TermDepth(handle->lterm);
      lweight = lweight*local->max_term_multiplier
                       *(TermIsAppliedFreeVar(handle->lterm) ? local->app_var_mult : 1);

      rweight = TermWeight(handle->rterm, local->vweight, local->fweight)
    - local->term_depth_multiplier*TermDepth(handle->rterm);

      if(!EqnIsOriented(handle))
      {
    rweight = rweight*local->max_term_multiplier;
      }
      weight = lweight+rweight*(TermIsAppliedFreeVar(handle->rterm) ? local->app_var_mult : 1);

      if(EqnIsPositive(handle))
      {
    weight = weight*local->pos_multiplier;
      }
      if(EqnIsMaximal(handle))
      {
    weight = weight*local->max_literal_multiplier;
      }
      res += weight;
   }
   return res;

}


/*-----------------------------------------------------------------------
//
// Function: NLWeightInit()
//
//   Initialize a WFCB for a Non-Linear Weight-Evaluation function, that
//   modifies a base Refinedweight by distinguishing linear and
//   non-linear variables.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p NLWeightInit(ClausePrioFun prio_fun, int fweight,
          int vlweight, int vweight, OCB_p ocb, double
          max_term_multiplier, double
          max_literal_multiplier, double
          pos_multiplier, double app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vlweight               = vlweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(NLWeightCompute, prio_fun,
                    VarWeightExit, data);
}

/*-----------------------------------------------------------------------
//
// Function: NLWeightParse()
//
//   Parse a NLWeight-Evaluation function.
//
// Global Variables: -
//
// Side Effects    : Reads input, memory management
//
/----------------------------------------------------------------------*/

WFCB_p NLWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
            state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight,vlweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vlweight = ParseInt(in);
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

   return NLWeightInit(prio_fun, fweight, vlweight, vweight, ocb,
             max_term_multiplier, max_literal_multiplier,
             pos_multiplier, app_var_mult);
}



/*-----------------------------------------------------------------------
//
// Function: NLWeightCompute()
//
//   Compute a non-linar weight.
//
// Global Variables: -
//
// Side Effects    : May orient clause
//
/----------------------------------------------------------------------*/

double NLWeightCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseNonLinearWeight(clause,
            local->max_term_multiplier,
            local->max_literal_multiplier,
            local->pos_multiplier,
            local->vlweight,
            local->vweight,
            local->fweight,
            local->app_var_mult,
            false);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PNRefinedWeightInit()
//
//   Return an initialized WFCB for PNRefinedWeight evaluation.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p PNRefinedWeightInit(ClausePrioFun prio_fun, int fweight,
            int vweight, int nfweight, int nvweight,
            OCB_p ocb, double max_term_multiplier,
            double max_literal_multiplier, double
            pos_multiplier, double app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->nfweight               = nfweight;
   data->nvweight               = nvweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(PNRefinedWeightCompute, prio_fun,
                    VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: PNRefinedWeightParse()
//
//   Parse a PNRefinedWeight definition
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p PNRefinedWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
             state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight,nfweight,nvweight;
   double pos_multiplier, max_term_multiplier, max_literal_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

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
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return PNRefinedWeightInit(prio_fun, fweight, vweight, nfweight,
               nvweight, ocb, max_term_multiplier,
               max_literal_multiplier, pos_multiplier, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: PNRefinedWeightCompute()
//
//   Compute an evaluation for a clause as in ClauseRefinedWeight, but
//   use different weights for function symbols/variables in
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/


double PNRefinedWeightCompute(void* data, Clause_p clause)
{
   Eqn_p  handle;
   double res = 0;
   VarWeightParam_p local = data;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   for(handle = clause->literals; handle; handle = handle->next)
   {
      if(EqnIsPositive(handle))
      {
         res += LiteralWeight(handle, local->max_term_multiplier,
                              local->max_literal_multiplier,
                              local->pos_multiplier,
                              local->vweight, local->fweight, local->app_var_mult,
                              false);
      }
      else
      {
         res += LiteralWeight(handle, local->max_term_multiplier,
                              local->max_literal_multiplier,
                              local->pos_multiplier,
                              local->nvweight, local->nfweight, local->app_var_mult,
                              false);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SymTypeWeightInit()
//
//   Return an initialized WFCB for SymTypeWeight evaluation. This
//   gives different weights to non-constant symbols, constant
//   symbols, predicate symbols, and variables.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p SymTypeWeightInit(ClausePrioFun prio_fun, int fweight,
          int vweight, int cweight, int pweight, OCB_p
          ocb, double max_term_multiplier, double
          max_literal_multiplier, double
          pos_multiplier, double app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->cweight                = cweight;
   data->pweight                = pweight;
   data->pos_multiplier         = pos_multiplier;
   data->max_term_multiplier    = max_term_multiplier;
   data->max_literal_multiplier = max_literal_multiplier;
   data->ocb                    = ocb;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(SymTypeWeightCompute, prio_fun,
                    VarWeightExit, data);
}

/*-----------------------------------------------------------------------
//
// Function: SymTypeWeightParse()
//
//   Parse a SymTypeWeight declaration, return a suitable WFCB.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p SymTypeWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int           fweight, vweight, cweight, pweight;
   double        max_term_multiplier, max_literal_multiplier,
                 pos_multiplier, app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   cweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   max_term_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   max_literal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return SymTypeWeightInit(prio_fun, fweight, vweight, cweight,
             pweight, ocb, max_term_multiplier,
             max_literal_multiplier, pos_multiplier, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: SymTypeWeightCompute()
//
//   Compute a symbol type based clause weight.
//
// Global Variables: -
//
// Side Effects    : May orient clause.
//
/----------------------------------------------------------------------*/

double SymTypeWeightCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   double           res;

   ClauseCondMarkMaximalTerms(local->ocb, clause);
   res = ClauseSymTypeWeight(clause,
              local->max_term_multiplier,
              local->max_literal_multiplier,
              local->pos_multiplier,
              local->vweight,
              local->fweight,
              local->cweight,
              local->pweight,
              local->app_var_mult);
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightAgeInit()
//
//   Return an initialized WFCB for ClauseWeightAge evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p ClauseWeightAgeInit(ClausePrioFun prio_fun, int fweight, int
            vweight, double pos_multiplier,
            double weight_multiplier,
            double app_var_mult)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->weight_multiplier      = weight_multiplier;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(ClauseWeightAgeCompute, prio_fun,
          VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightAgeParse()
//
//   Parse a clauseweight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p ClauseWeightAgeParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier, weight_multiplier,
          app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   weight_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ClauseWeightAgeInit(prio_fun, fweight, vweight,
               pos_multiplier, weight_multiplier, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightAgeCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double ClauseWeightAgeCompute(void* data, Clause_p clause)
{
   VarWeightParam_p local = data;
   return (local->weight_multiplier
      *
      ClauseWeight(clause, 1, 1,
         local->pos_multiplier,
         local->vweight,
         local->fweight,
         local->app_var_mult,
         false))
      +clause->create_date;
}



/*-----------------------------------------------------------------------
//
// Function: StaggeredWeightInit()
//
//   Initialize a staggered evaluation function (to replace FIFO).
//   Assign weight
//   (int)(ClauseStandardWeight(clause)/
//   (max(ClauseStandardWeight(initial_clause_set)*stagger_factor)).
//   Precedence within each class is by the tie-breaking fifo.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

WFCB_p StaggeredWeightInit(ClausePrioFun prio_fun,
            double stagger_factor, ClauseSet_p axioms)
{
   VarWeightParam_p data = VarWeightParamCellAlloc();
   long clause_max_size = ClauseSetMaxStandardWeight(axioms);

   data->stagger_limit = MAX(stagger_factor*clause_max_size,1);
   return WFCBAlloc(StaggeredWeightCompute, prio_fun, VarWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: StaggeredWeightParse()
//
//   Parse a staggered weight evaluation function.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/


WFCB_p StaggeredWeightParse(Scanner_p in, OCB_p ocb, ProofState_p
           state)
{
   ClausePrioFun prio_fun;
   double stagger_factor;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   stagger_factor = ParseFloat(in);
   AcceptInpTok(in, CloseBracket);

   return StaggeredWeightInit(prio_fun, stagger_factor, state->axioms);
}


/*-----------------------------------------------------------------------
//
// Function: StaggeredWeightCompute()
//
//   Compute the staggered weight of a clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double StaggeredWeightCompute(void* data, Clause_p clause)
{
   long res, weight;
   VarWeightParam_p local = data;

   weight = ClauseStandardWeight(clause);
   res = weight/local->stagger_limit;
   return (double)res;
}




/*-----------------------------------------------------------------------
//
// Function: VarWeightExit()
//
//   Free the data entry in a varweight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void VarWeightExit(void* data)
{
   VarWeightParam_p junk = data;

   VarWeightParamCellFree(junk);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
