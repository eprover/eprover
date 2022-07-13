/*-----------------------------------------------------------------------

  File  : che_simweight.c

  Author: Stephan Schulz

  Contents

  Functions realising clause evaluation with similarities weights.

  Copyright 1998-1999, 2022 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Sun Jun 28 18:18:00 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_simweight.h"


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
// Function: sim_eqn_weight()
//
//   Evaluate am equation by computing term similarity of both sides.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double sim_eqn_weight(Eqn_p eqn, SimParam_p parms)
{
   double   clash_weight = 0.0;
   PStack_p stack = PStackAlloc();
   Term_p   lside, rside;
   int      i;

   lside = eqn->lterm;
   rside = eqn->rterm;
   PStackPushP(stack, lside);
   PStackPushP(stack, rside);

   while(!PStackEmpty(stack))
   {
      rside = PStackPopP(stack);
      lside = PStackPopP(stack);

      if(lside->f_code == rside->f_code)
      {
         for(i=0; i<lside->arity; i++)
         {
            PStackPushP(stack, lside->args[i]);
            PStackPushP(stack, rside->args[i]);
         }
      }
      else
      {
         if(TermIsFreeVar(lside))
         {
            if(TermIsFreeVar(rside))
            {
               clash_weight += parms->var_var_clash;
            }
            else
            {
               clash_weight += parms->var_term_clash;
            }
         }
         else
         {
            if(TermIsFreeVar(rside))
            {
               clash_weight += parms->var_term_clash;
            }
            else
            {
               clash_weight += parms->term_term_clash *
                  (TermWeight(lside, 1, 1)+TermWeight(rside, 1, 1));
            }
         }
      }
   }
   PStackFree(stack);
   return clash_weight;
}


/*-----------------------------------------------------------------------
//
// Function: sim_weight()
//
//   Evaluate a clause by computing term similarity of equations.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double sim_weight(Clause_p clause, SimParam_p parms)
{
   Eqn_p  handle = clause->literals;
   double res    = 0.0;

   while(handle)
   {
      res += sim_eqn_weight(handle, parms);
      handle = handle->next;
   }
   return res*5+ClauseWeight(clause, 1, 1, 1, 1, 2, parms->app_var_mult, false);
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: SimWeightInit()
//
//   Return an initialized WFCB for SimWeight evaluation.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p SimWeightInit(ClausePrioFun prio_fun, double equal_weight, double
                     var_var_clash, double var_term_clash, double
                     term_term_clash, double app_var_mult)
{
   SimParam_p data = SimParamCellAlloc();

   data->equal_weight    = equal_weight;
   data->equal_weight    = equal_weight;
   data->var_var_clash   = var_var_clash;
   data->var_term_clash  = var_term_clash;
   data->term_term_clash = term_term_clash;
   data->app_var_mult    = app_var_mult;

   return WFCBAlloc(SimWeightCompute, prio_fun, SimWeightExit, data);
}



/*-----------------------------------------------------------------------
//
// Function: SimWeightParse()
//
//   Parse a simweight-definition.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

WFCB_p SimWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   double equal_weight, var_var_clash, var_term_clash,
      term_term_clash, app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   equal_weight = ParseFloat(in);
   AcceptInpTok(in, Comma);
   var_var_clash = ParseFloat(in);
   AcceptInpTok(in, Comma);
   var_term_clash = ParseFloat(in);
   AcceptInpTok(in, Comma);
   term_term_clash = ParseFloat(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return SimWeightInit(prio_fun, equal_weight, var_var_clash,
                        var_term_clash, term_term_clash, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: SimWeightCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double SimWeightCompute(void* data, Clause_p clause)
{
   SimParam_p    local = data;

   return sim_weight(clause, local);
}


/*-----------------------------------------------------------------------
//
// Function: SimWeightExit()
//
//   Free the data entry in a Simweight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void SimWeightExit(void* data)
{
   SimParamCellFree(data);
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
