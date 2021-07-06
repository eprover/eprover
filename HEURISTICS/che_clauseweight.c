/*-----------------------------------------------------------------------

File  : che_clauseweight.c

Author: Stephan Schulz

Contents

  Functions realising clause evaluation with clause weights.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jun  5 22:51:52 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_clauseweight.h"


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
// Function: uniq_term_weight()
//
//   Return the uniqweight of a term.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double uniq_term_weight(Term_p term)
{
   double weight;

   if(TermIsFreeVar(term))
   {
      weight = 3.0;
   }
   else
   {
      int i;

      weight = pow(5,term->arity);
      for(i=0; i< term->arity; i++)
      {
         weight+= 2*uniq_term_weight(term->args[i]);
      }
   }
   return weight;
}

/*-----------------------------------------------------------------------
//
// Function: uniq_eqn_weight()
//
//   Return the uniqweight of a equation.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double uniq_eqn_weight(Eqn_p handle)
{
   double multiplier = EqnIsPositive(handle)?7:11;

   return multiplier*(uniq_term_weight(handle->lterm)
                      + uniq_term_weight(handle->rterm));
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightInit()
//
//   Return an initialized WFCB for ClauseWeight evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p ClauseWeightInit(ClausePrioFun prio_fun, int fweight, int
         vweight, double pos_multiplier, double app_var_mult)
{
   WeightParam_p data = WeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(ClauseWeightCompute, prio_fun,
          ClauseWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightParse()
//
//   Parse a clauseweight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p ClauseWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier;
   double app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return ClauseWeightInit(prio_fun, fweight, vweight,
            pos_multiplier, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double ClauseWeightCompute(void* data, Clause_p clause)
{
   WeightParam_p local = data;
   return ClauseWeight(clause, 1, 1,
             local->pos_multiplier,
             local->vweight,
             local->fweight,
             local->app_var_mult,
             false);
}


/*-----------------------------------------------------------------------
//
// Function: ClauseWeightExit()
//
//   Free the data entry in a clauseweight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void ClauseWeightExit(void* data)
{
   WeightParamCellFree(data);
}

/*-----------------------------------------------------------------------
//
// Function: LMaxWeightInit()
//
//   Return an initialized WFCB for LMaxWeight evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p LMaxWeightInit(ClausePrioFun prio_fun, int fweight, int
         vweight, double pos_multiplier, double app_var_mult)
{
   WeightParam_p data = WeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(LMaxWeightCompute, prio_fun,
          ClauseWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: LMaxWeightParse()
//
//   Parse a LMaxweight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p LMaxWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier;
   double app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return LMaxWeightInit(prio_fun, fweight, vweight,
          pos_multiplier, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: LMaxWeightCompute()
//
//   Compute an LMax evaluation for a clause. Each literal is weigthed
//   with the weight of its heaviest term.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double LMaxWeightCompute(void* data, Clause_p clause)
{
   WeightParam_p local = data;
   double res = 0, tmp;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = EqnMaxWeight(handle, local->vweight, local->fweight, local->app_var_mult);
      if(EqnIsPositive(handle))
      {
         tmp = tmp*local->pos_multiplier;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: CMaxWeightInit()
//
//   Return an initialized WFCB for CMaxWeight evaluation.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p CMaxWeightInit(ClausePrioFun prio_fun, int fweight, int
         vweight, double pos_multiplier, double app_var_mult)
{
   WeightParam_p data = WeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->app_var_mult           = app_var_mult;

   return WFCBAlloc(CMaxWeightCompute, prio_fun,
          ClauseWeightExit, data);
}


/*-----------------------------------------------------------------------
//
// Function: CMaxWeightParse()
//
//   Parse a CMaxweight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p CMaxWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight;
   double pos_multiplier;
   double app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return CMaxWeightInit(prio_fun, fweight, vweight,
            pos_multiplier, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: CMaxWeightCompute()
//
//   Compute an evaluation for a clause, multiplying the weight of the
//   largest term by the number of literals.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double CMaxWeightCompute(void* data, Clause_p clause)
{
   WeightParam_p local = data;
   double res = 0, tmp;
   Eqn_p handle;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      tmp = EqnMaxWeight(handle, local->vweight, local->fweight, local->app_var_mult);
      res = MAX(res,tmp);
   }
   return clause->pos_lit_no*res*local->pos_multiplier +
          clause->neg_lit_no*res;
}


/*-----------------------------------------------------------------------
//
// Function: UniqWeightInit()
//
//   Return an initialized WFCB for UniqWeight evaluation. UniqWeight
//   is designed to return a "maximally unique" weight that is
//   invariant with respect to function symbol renaming, reordering
//   and so on.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p UniqWeightInit(ClausePrioFun prio_fun)
{
   return WFCBAlloc(UniqWeightCompute, prio_fun,
          TrivialWeightExit, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: UniqWeightParse()
//
//   Parse a uniqweight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p UniqWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, CloseBracket);

   return UniqWeightInit(prio_fun);
}

/*-----------------------------------------------------------------------
//
// Function: UniqWeightCompute()
//
//   Compute a hopefully uniq weight for each clause (see above)
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double UniqWeightCompute(void* data, Clause_p clause)
{
   Eqn_p  handle;
   double weight = 0;

   for(handle = clause->literals; handle; handle = handle->next)
   {
      weight+= uniq_eqn_weight(handle);
   }
   return weight;
}


/*-----------------------------------------------------------------------
//
// Function: DefaultWeightInit()
//
//   Return an initialized WFCB for DefaultWeight evaluation. This
//   uses the precomputed default clause weight for evaluation.
//
// Global Variables: -
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p DefaultWeightInit(ClausePrioFun prio_fun)
{
   return WFCBAlloc(DefaultWeightCompute, prio_fun,
          TrivialWeightExit, NULL);
}


/*-----------------------------------------------------------------------
//
// Function: DefaultWeightParse()
//
//   Parse a default weight-definition.
//
// Global Variables: -
//
// Side Effects    : Memory operations, Input
//
/----------------------------------------------------------------------*/

WFCB_p DefaultWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, CloseBracket);

   return DefaultWeightInit(prio_fun);
}

/*-----------------------------------------------------------------------
//
// Function: DefaultWeightCompute()
//
//   Compute return the default weight.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

double DefaultWeightCompute(void* data, Clause_p clause)
{
   return ClauseStandardWeight(clause);
}

/*-----------------------------------------------------------------------
//
// Function: TrivialWeightExit()
//
//   Do nothing with the correct argument (for evaluation functions
//   that do not need to store any data).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void TrivialWeightExit(void* data)
{
   assert(data==NULL);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
