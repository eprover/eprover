/*-----------------------------------------------------------------------

  File  : che_gdweight.c

  Author: Stephan Schulz

  Contents

  Evaluation of a clause by E's version of TWEE-inspired goal-direced
  weight. Conjecture ground terms get a lower (better) weight here.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Fri Jun  5 22:51:52 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_gdweight.h"


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
// Function: gd_term_weight()
//
//   Compute the weight of a term, counting variables as vweight and
//   function symbols as fweight. Conjecture ground terms are treated
//   special.
//
// Global Variables: -
//
// Side Effects    : Memory operations for the stack used.
//
/----------------------------------------------------------------------*/

long gd_term_weight(Term_p term, long vweight, long fweight,
                    double goal_multiplier, long goal_const)
{
   long res = 0;

   if(TermIsGround(term)&&TermCellQueryProp(term, TPIsConjectureTerm))
   {
      if(goal_multiplier == 0)
      {
         res = goal_const;
      }
      else
      {
         res = goal_const+goal_multiplier*(TermWeight(term, fweight, vweight));
      }
   }
   else
   {
      if(TermIsFreeVar(term) ||
         (TermIsAppliedFreeVar(term) && NormalizePatternAppVar(TermGetBank(term), term)))
      {
         // if variable is of the form X xn where xn are bound variables -->
         // then it is counted as a simple variable
         res += vweight;
      }
      else
      {
         // lambdas (and corresponding bound var) are ignored.
         res += fweight*((TermIsPhonyApp(term) || TermIsDBLambda(term)) ? 0 : 1);
         for(int i=(TermIsDBLambda(term)?1:0); i<term->arity; i++)
         {
            res += gd_term_weight(term->args[i], vweight, fweight,
                                  goal_multiplier, goal_const);
         }
      }
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: gd_literal_weight()
//
//   Return weight of a literal. Atoms are weight without equational
//   encoding. pos_multiplier is applied to positive literals.
//   Applied variable's weights are multiplied by app_var_mult.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double  gd_literal_weight(Eqn_p eq, double pos_multiplier, long vweight,
                          long fweight, double goal_multiplier,
                          long goal_const, double app_var_mult)
{
   double res = 0, lweight;
   long tmp;

   if(EqnIsEquLit(eq))
   {
      res = (double)gd_term_weight(eq->rterm, vweight, fweight,
                                   goal_multiplier, goal_const);
      //printf("Rterm: %f\n", res);
      res = TERM_APPLY_APP_VAR_MULT(res, eq->rterm, app_var_mult);
      res += fweight; /* Count the equal-predicate */
   }
   tmp = gd_term_weight(eq->lterm, vweight, fweight,
                        goal_multiplier, goal_const);
   lweight = (double)tmp;
   lweight = TERM_APPLY_APP_VAR_MULT(lweight, eq->lterm, app_var_mult);
   //printf("Lterm: %f = %ld\n", lweight, tmp);

   res += lweight;

   if(EqnIsPositive(eq))
   {
      res = res*pos_multiplier;
   }
   return res;
}



/*-----------------------------------------------------------------------
//
// Function: gd_clause_weight()
//
//   Compute the weight of a clause by counting function symbols and
//   variables and applying various modifiers.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double gd_clause_weight(Clause_p clause, double
                        pos_multiplier, long vweight, long fweight,
                        double goal_multiplier, long goal_const,
                        double app_var_mult)
{
   Eqn_p  handle;
   double res = 0;

   //ClauseTSTPPrint(stdout, clause, true, true);
   //printf("\n");
   for(handle = clause->literals; handle; handle = handle->next)
   {
      res += gd_literal_weight(handle, pos_multiplier,
                               vweight, fweight,
                               goal_multiplier, goal_const,
                               app_var_mult);

   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: initialize_goal_terms()
//
//   Set TPIsConjectureTerm in all terms occuring in elements from
//   axioms with type negated_conjecture.
//
// Global Variables: -
//
// Side Effects    : Changes properties of shared terms.
//
/----------------------------------------------------------------------*/

void initialize_goal_terms(ClauseSet_p axioms)
{
   Clause_p handle;

   for(handle=axioms->anchor->succ;
       handle!=axioms->anchor;
       handle = handle->succ)
   {
      if(ClauseQueryTPTPType(handle)==CPTypeNegConjecture)
      {
         ClauseTermSetProp(handle, TPIsConjectureTerm);
      }
   }
}



/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: GDClauseWeightInit()
//
//   Return an initialized WFCB for GDClauseWeight evaluation.
//
//   The new parameters are goal_multiplier and goal_const. Goal terms
//   are evaluated as (tw*goal_multiplier)+goal_const. To mimmic TWEE,
//   goal_multiplier should be 0.0 and goal_const should be fweight
//   (i.e. goal terms have the same weight as a normal constant).
//   To model normal clauseweight, set goal_multiplier to 1.0 and
//   goal_const to 0. Interesting things may happen with other
//   settings (or not).
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

WFCB_p GDClauseWeightInit(ClausePrioFun prio_fun, ClauseSet_p axioms,
                          int fweight, int
                          vweight, double pos_multiplier,
                          double goal_multiplier, long goal_const,
                          double app_var_mult)
{
   GDWeightParam_p data = GDWeightParamCellAlloc();

   data->fweight                = fweight;
   data->vweight                = vweight;
   data->pos_multiplier         = pos_multiplier;
   data->goal_multiplier        = goal_multiplier;
   data->goal_const             = goal_const;
   data->app_var_mult           = app_var_mult;
   data->goal_terms_initialized = false;
   data->axioms                 = axioms;

   return WFCBAlloc(GDClauseWeightCompute, prio_fun,
                    GDClauseWeightExit, data);
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

WFCB_p GDClauseWeightParse(Scanner_p in, OCB_p ocb, ProofState_p state)
{
   ClausePrioFun prio_fun;
   int fweight, vweight, goal_const;
   double pos_multiplier, goal_multiplier;
   double app_var_mult = APP_VAR_MULT_DEFAULT;

   AcceptInpTok(in, OpenBracket);
   prio_fun = ParsePrioFun(in);
   AcceptInpTok(in, Comma);
   fweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   vweight = ParseInt(in);
   AcceptInpTok(in, Comma);
   pos_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   goal_multiplier = ParseFloat(in);
   AcceptInpTok(in, Comma);
   goal_const = ParseInt(in);

   PARSE_OPTIONAL_AV_PENALTY(in, app_var_mult);

   AcceptInpTok(in, CloseBracket);

   return GDClauseWeightInit(prio_fun, state->axioms, fweight, vweight,
                             pos_multiplier, goal_multiplier,
                             goal_const, app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: GDClauseWeightCompute()
//
//   Compute an evaluation for a clause.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

double GDClauseWeightCompute(void* data, Clause_p clause)
{
   GDWeightParam_p local = data;
   if(!local->goal_terms_initialized)
   {
      initialize_goal_terms(local->axioms);
      local->goal_terms_initialized = true;
   }

   return gd_clause_weight(clause,
                           local->pos_multiplier,
                           local->vweight,
                           local->fweight,
                           local->goal_multiplier,
                           local->goal_const,
                           local->app_var_mult);
}


/*-----------------------------------------------------------------------
//
// Function: GDClauseWeightExit()
//
//   Free the data entry in a clauseweight WFCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

void GDClauseWeightExit(void* data)
{
   GDWeightParamCellFree(data);
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
