/*-----------------------------------------------------------------------

  File  : che_clausesetfeatures.c

  Author: Stephan Schulz

  Contents

  Functions for calculating certain features of clause sets.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.


  Created: Tue Sep 29 02:50:34 MET DST 1998

  -----------------------------------------------------------------------*/

#include "che_clausesetfeatures.h"
#include <sys/wait.h>

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
// Function: SpecLimitsAlloc()
//
//   Allocate an initialized SpecLimitsCell.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

SpecLimits_p SpecLimitsAlloc(void)
{
   SpecLimits_p handle = SpecLimitsCellAlloc();

   handle->ngu_absolute           = NGU_ABSOLUTE;
   if(NGU_ABSOLUTE)
   {
      handle->ngu_few_limit          = NGU_FEW_ABSDEFAULT;
      handle->ngu_many_limit         = NGU_MANY_ABSDEFAULT;
   }
   else
   {
      handle->ngu_few_limit          = NGU_FEW_DEFAULT;
      handle->ngu_many_limit         = NGU_MANY_DEFAULT;
   }
   handle->gpc_absolute           = GPC_ABSOLUTE;
   if(GPC_ABSOLUTE)
   {
      handle->gpc_few_limit          = GPC_FEW_ABSDEFAULT;
      handle->gpc_many_limit         = GPC_MANY_ABSDEFAULT;
   }
   else
   {
      handle->gpc_few_limit          = GPC_FEW_DEFAULT;
      handle->gpc_many_limit         = GPC_MANY_DEFAULT;
   }
   handle->ax_some_limit          = AX_SOME_DEFAULT ;
   handle->ax_many_limit          = AX_MANY_DEFAULT ;
   handle->lit_some_limit         = LIT_SOME_DEFAULT;
   handle->lit_many_limit         = LIT_MANY_DEFAULT;
   handle->term_medium_limit      = TERM_MED_DEFAULT;
   handle->term_large_limit       = TERM_LARGE_DEFAULT;
   handle->far_sum_medium_limit   = FAR_SUM_MED_DEFAULT  ;
   handle->far_sum_large_limit    = FAR_SUM_LARGE_DEFAULT;
   handle->depth_medium_limit     = DEPTH_MEDIUM_DEFAULT;
   handle->depth_deep_limit       = DEPTH_DEEP_DEFAULT;
   handle->symbols_medium_limit   = SYMBOLS_MEDIUM_DEFAULT;
   handle->symbols_large_limit    = SYMBOLS_LARGE_DEFAULT;

   handle->predc_medium_limit     = PREDC_MEDIUM_DEFAULT;
   handle->predc_large_limit      = PREDC_LARGE_DEFAULT;
   handle->pred_medium_limit      = PRED_MEDIUM_DEFAULT;
   handle->pred_large_limit       = PRED_LARGE_DEFAULT;
   handle->func_medium_limit      = FUNC_MEDIUM_DEFAULT;
   handle->func_large_limit       = FUNC_LARGE_DEFAULT;
   handle->fun_medium_limit       = FUN_MEDIUM_DEFAULT;
   handle->fun_large_limit        = FUN_LARGE_DEFAULT;

   handle->order_medium_limit     = ORDER_MEDIUM_DEFAULT;
   handle->order_large_limit      = ORDER_LARGE_DEFAULT;
   handle->num_of_defs_medium_limit = DEFS_MEDIUM_DEFAULT;
   handle->num_of_defs_large_limit = DEFS_LARGE_DEFAULT;
   handle->perc_form_defs_medium_limit = DEFS_PERC_MEDIUM_DEFAULT;
   handle->perc_form_defs_large_limit = DEFS_PERC_MEDIUM_DEFAULT;
   handle->perc_app_lits_medium_limit = PERC_APPLIT_MEDIUM_DEFAULT;
   handle->perc_app_lits_large_limit = PERC_APPLIT_LARGE_DEFAULT;
   handle->num_of_lams_medium_limit = NUM_LAMS_MEDIUM_DEFAULT;
   handle->num_of_lams_large_limit = NUM_LAMS_LARGE_DEFAULT;


   return handle;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountGoals()
//
//   Count number of goal clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountGoals(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsGoal(handle))
      {
         res++;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountUnit()
//
//   Count the unit clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountUnit(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsUnit(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountUnitGoals()
//
//   Count the unit goal clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountUnitGoals(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsUnit(handle)&&ClauseIsGoal(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountHorn()
//
//   Count the unit clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountHorn(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsHorn(handle))
      {
         res++;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountHornGoals()
//
//   Count the unit clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountHornGoals(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsHorn(handle)&&ClauseIsGoal(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountEquational()
//
//   Count number of clauses with at least one equational literal.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountEquational(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsEquational(handle))
      {
         res++;
      }
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountPureEquational()
//
//   Count number of clauses which have only equational literals.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountPureEquational(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsPureEquational(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountPosUnits()
//
//   Count number of positive unit clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountPosUnits(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsDemodulator(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountGroundGoals()
//
//   Count number of ground goal clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountGroundGoals(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsGoal(handle) && ClauseIsGround(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountGround()
//
//   Count number of ground clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountGround(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsGround(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountGroundUnitAxioms()
//
//   Count number of positive ground unit clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountGroundUnitAxioms(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsDemodulator(handle) && ClauseIsGround(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountGroundPositiveAxioms()
//
//   Count number of positive ground clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountGroundPositiveAxioms(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsPositive(handle) && ClauseIsGround(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountPositiveAxioms()
//
//   Count number of positive ground clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountPositiveAxioms(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsPositive(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountRangeRestricted()
//
//   Count number of positive ground clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountRangeRestricted(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsRangeRestricted(handle))
      {
         res++;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetNonGoundAxiomPart()
//
//   Return the percentage of non-ground clauses among the unit
//   clauses (0 if no unit clauses exist).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double ClauseSetNonGoundAxiomPart(ClauseSet_p set)
{
   long tmp = ClauseSetCountUnitAxioms(set);

   if(tmp == 0)
   {
      return 0.0;
   }
   return (tmp-ClauseSetCountGroundUnitAxioms(set))/(double)tmp;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCollectArityInformation()
//
//   Collect information about the arities of function and predicate
//   symbol arities. Average and sum for function symbols does not
//   include constants, it does for predicate symbols. Equality is not
//   counted, Returns number of function symbol constants.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

long ClauseSetCollectArityInformation(ClauseSet_p set, Sig_p sig,
                                      int *max_fun_arity,
                                      int *avg_fun_arity,
                                      int *sum_fun_arity,
                                      int *max_pred_arity,
                                      int *avg_pred_arity,
                                      int *sum_pred_arity,
                                      int *non_const_funs,
                                      int *non_const_preds)
{
   int max_f_arity = 0,
      sum_f_arity = 0,
      f_count = 0,
      c_count = 0,
      non_const_p = 0;
   int max_p_arity = 0,
      sum_p_arity = 0,
      p_count = 0;
   long  array_size = sizeof(long)*(sig->f_count+1);
   long *dist_array = SizeMalloc(array_size);
   FunCode i;

   for(i=1; i<= sig->f_count; i++)
   {
      dist_array[i] = 0;
   }
   ClauseSetAddSymbolDistribution(set, dist_array);

   for(i=1; i<= sig->f_count; i++)
   {
      if(!SigIsSpecial(sig, i)&&dist_array[i])
      {
         short arity = SigFindArity(sig, i);
         if(SigIsPredicate(sig, i))
         {
            max_p_arity = MAX(arity, max_p_arity);
            sum_p_arity += arity;
            p_count++;
            if(arity)
            {
               non_const_p++;
            }
         }
         else
         {
            if(arity)
            {
               max_f_arity = MAX(arity, max_f_arity);
               sum_f_arity += arity;
               f_count++;
            }
            else
            {
               c_count++;
            }
         }
      }
   }
   SizeFree(dist_array, array_size);

   *max_fun_arity   = max_f_arity;
   *avg_fun_arity   = f_count?sum_f_arity/f_count:0;
   *sum_fun_arity   = sum_f_arity;
   *max_pred_arity  = max_p_arity;
   *avg_pred_arity  = p_count?sum_p_arity/p_count:0;
   *sum_pred_arity  = sum_p_arity;
   *non_const_funs  = f_count;
   *non_const_preds = non_const_p;

   return c_count;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountMaximalTerms()
//
//   Count the number of maximal terms in maximal literals in clauses
//   in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountMaximalTerms(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClauseCountMaximalTerms(handle);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountMaximalLiterals()
//
//   Count the number of maximal literals in clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountMaximalLiterals(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClauseCountMaximalLiterals(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountVariables()
//
//   Count the number of variables in a clause set, where variables in
//   different clauses are considered to be distinct.
//
// Global Variables: -
//
// Side Effects    : By calling ClauseCountVariableSet
//
/----------------------------------------------------------------------*/

long ClauseSetCountVariables(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClauseCountVariableSet(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountSingletons()
//
//   Count the number of singletons in a clause set, where variables in
//   different clauses are considered to be distinct.
//
// Global Variables: -
//
// Side Effects    : By calling ClauseCountSingletonSet
//
/----------------------------------------------------------------------*/

long ClauseSetCountSingletons(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClauseCountSingletonSet(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetTPTPDepthInfoAdd()
//
//   Add the depth information in TPTP interpretation to the
//   variables. See che_clausefeatures.c for more.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

long ClauseSetTPTPDepthInfoAdd(ClauseSet_p set, long* depthmax, long*
                               depthsum, long* count)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      ClauseTPTPDepthInfoAdd(handle, depthmax, depthsum, count);
   }
   return *depthmax;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountUnorientableLiterals()
//
//   Count the number of Unorientable literals in clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountUnorientableLiterals(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClauseCountUnorientableLiterals(handle);
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetCountEqnLiterals()
//
//   Count the number of equational literals in clauses in set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountEqnLiterals(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClausePropLitNumber(handle, EPIsEquLiteral);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetMaxStandardWeight()
//
//   Return the standard weight of the largest clause in set (or -1 if
//   set is empty).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetMaxStandardWeight(ClauseSet_p set)
{
   long res = -1;
   Clause_p handle = ClauseSetFindMaxStandardWeight(set);

   if(handle)
   {
      res = ClauseStandardWeight(handle);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetTermCells()
//
//   Return the number of term positions in the clause set.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetTermCells(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res += ClauseWeight(handle, 1, 1, 1, 1, 1, 1, false);
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetMaxLiteralNumber()
//
//   Return the length of the longest clause.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetMaxLiteralNumber(ClauseSet_p set)
{
   Clause_p handle;
   long     res = 0;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      res = MAX(res, ClauseLiteralNumber(handle));
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: SpecFeaturesCompute()
//
//   Compute all relevant features for a set of clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SpecFeaturesCompute(SpecFeature_p features, ClauseSet_p set,
                         FormulaSet_p fset, FormulaSet_p farch, TB_p bank)
{
   long tmp, count;
   Sig_p sig = bank->sig;
   features->clauses          = set->members;
   features->goals            = ClauseSetCountGoals(set);
   features->axioms           = features->clauses-features->goals;

   features->literals         = set->literals;
   features->term_cells       = ClauseSetTermCells(set);

   tmp = 0;
   count = 0;
   features->clause_max_depth = 0;
   ClauseSetTPTPDepthInfoAdd(set,
                             &(features->clause_max_depth),
                             &tmp,
                             &count);
   features->clause_avg_depth = count?tmp/count:0;

   features->unit             = ClauseSetCountUnit(set);
   features->unitgoals        = ClauseSetCountUnitGoals(set);
   features->unitaxioms       = features->unit-features->unitgoals;

   features->horn             = ClauseSetCountHorn(set);
   features->horngoals        = ClauseSetCountHornGoals(set);
   features->hornaxioms       = features->horn-features->horngoals;

   features->eq_clauses       = ClauseSetCountEquational(set);
   features->peq_clauses      = ClauseSetCountPureEquational(set);
   features->groundunitaxioms = ClauseSetCountGroundUnitAxioms(set);
   features->groundgoals      = ClauseSetCountGroundGoals(set);
   features->positiveaxioms   = ClauseSetCountPositiveAxioms(set);
   features->groundpositiveaxioms = ClauseSetCountGroundPositiveAxioms(set);
   features->fun_const_count   =
      ClauseSetCollectArityInformation(set, sig,
                                       &(features->max_fun_arity),
                                       &(features->avg_fun_arity),
                                       &(features->sum_fun_arity),
                                       &(features->max_pred_arity),
                                       &(features->avg_pred_arity),
                                       &(features->sum_pred_arity),
                                       &(features->fun_nonconst_count),
                                       &(features->pred_nonconst_count));

   features->goals_are_ground = (features->groundgoals ==
                                 features->goals);

   if(features->unitaxioms == features->axioms)
   {
      features->axiomtypes = SpecUnit;
   }
   else if(features->hornaxioms == features->axioms)
   {
      features->axiomtypes = SpecHorn;
   }
   else
   {
      features->axiomtypes = SpecGeneral;
   }

   if(features->unitgoals == features->goals)
   {
      features->goaltypes = SpecUnit;
   }
   else if(features->horngoals == features->goals)
   {
      features->goaltypes = SpecHorn;
   }
   else
   {
      features->goaltypes = SpecGeneral;
   }

   if(features->peq_clauses == features->clauses)
   {
      features->eq_content = SpecPureEq;
   }
   else if(features->eq_clauses)
   {
      features->eq_content = SpecSomeEq;
   }
   else
   {
      features->eq_content = SpecNoEq;
   }

   switch(features->max_fun_arity)
   {
   case 0:
         features->max_fun_ar_class = SpecArity0;
         break;
   case 1:
         features->max_fun_ar_class = SpecArity1;
         break;
   case 2:
         features->max_fun_ar_class = SpecArity2;
         break;
   default:
         features->max_fun_ar_class = SpecArity3Plus;
         break;
   }
   switch(features->avg_fun_arity)
   {
   case 0:
         features->avg_fun_ar_class = SpecArity0;
         break;
   case 1:
         features->avg_fun_ar_class = SpecArity1;
         break;
   case 2:
         features->avg_fun_ar_class = SpecArity2;
         break;
   default:
         features->avg_fun_ar_class = SpecArity3Plus;
         break;
   }
   features->ng_unit_axioms_part = features->unitaxioms?
      ((double)(features->unitaxioms-features->groundunitaxioms)/
       (double)(features->unitaxioms))
      :0.0;

   features->ground_positive_axioms_part = features->positiveaxioms?
      ((double)(features->groundpositiveaxioms)/
       (double)(features->positiveaxioms))
      :0.0;

   /* all ho features computed below */
   features->num_of_definitions = -1;
   ClauseSetComputeHOFeatures(set, sig,
                              &(features->has_ho_features),
                              &(features->order),
                              &(features->quantifies_booleans),
                              &(features->has_defined_choice),
                              &(features->perc_of_appvar_lits));
   // overwriting order as different clausifications can influence it.
   features->order = 1;
   features->goal_order = 1;
   FormulaSet_p sets[2] = {farch, fset};
   for(int i=0; i<2; i++)
   {
      if(sets[i])
      {
         for(WFormula_p f = sets[i]->anchor->succ; f != sets[i]->anchor; f = f->succ)
         {
            int ord = TermComputeOrder(f->terms->sig, f->tformula);
            features->order = MAX(features->order, ord);
            if(FormulaQueryType(f) == CPTypeConjecture ||
               FormulaQueryType(f) == CPTypeNegConjecture ||
               FormulaQueryType(f) == CPTypeHypothesis )
            {
               features->goal_order = MAX(features->goal_order, ord);
            }
         }
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: SpecFeaturesAddEval()
//
//   Add the cheap, subjective things to a SpecFeatureCell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define ADJUST_FOR_HO(limit, scale) (limit)

void SpecFeaturesAddEval(SpecFeature_p features, SpecLimits_p limits)
{

   features->goals_are_ground = (features->groundgoals ==
                                 features->goals);

   if(limits->ngu_absolute)
   {
      features->ng_unit_content = SpecFewPosNonGroundUnits;
      if((features->unitaxioms-features->groundunitaxioms)>
         limits->ngu_few_limit)
      {
         features->ng_unit_content = SpecSomePosNonGroundUnits;
      }
      if((features->unitaxioms-features->groundunitaxioms)>
         limits->ngu_many_limit)
      {
         features->ng_unit_content = SpecManyPosNonGroundUnits;
      }
   }
   else
   {
      if(features->ng_unit_axioms_part<=limits->ngu_few_limit)
      {
         features->ng_unit_content = SpecFewPosNonGroundUnits;
      }
      else if(features->ng_unit_axioms_part>=limits->ngu_many_limit)
      {
         features->ng_unit_content = SpecManyPosNonGroundUnits;
      }
      else
      {
         features->ng_unit_content = SpecSomePosNonGroundUnits;
      }
   }

   if(limits->gpc_absolute)
   {
      features->ground_positive_content = SpecFewPosGround;
      if(features->groundpositiveaxioms > limits->gpc_few_limit)
      {
         features->ground_positive_content = SpecSomePosGround;
      }
      if(features->groundpositiveaxioms > limits->gpc_many_limit)
      {
         features->ground_positive_content = SpecManyPosGround;
      }
   }
   else
   {
      if(features->ground_positive_axioms_part<=limits->gpc_few_limit)
      {
         features->ground_positive_content = SpecFewPosGround;
      }
      else if(features->ground_positive_axioms_part>=limits->gpc_many_limit)
      {
         features->ground_positive_content = SpecManyPosGround;
      }
      else
      {
         features->ground_positive_content = SpecSomePosGround;
      }
   }

   if(features->clauses < ADJUST_FOR_HO(limits->ax_some_limit, 5))
   {
      features->set_clause_size = SpecFewAxioms;
   }
   else if(features->clauses < ADJUST_FOR_HO(limits->ax_many_limit, 7))
   {
      features->set_clause_size = SpecSomeAxioms;
   }
   else
   {
      features->set_clause_size = SpecManyAxioms;
   }

   if(features->literals < ADJUST_FOR_HO(limits->lit_some_limit, 5))
   {
      features->set_literal_size = SpecFewLiterals;
   }
   else if(features->literals < ADJUST_FOR_HO(limits->lit_many_limit, 7))
   {
      features->set_literal_size = SpecSomeLiterals;
   }
   else
   {
      features->set_literal_size = SpecManyLiterals;
   }

   if(features->term_cells < limits->term_medium_limit)
   {
      features->set_termcell_size = SpecSmallTerms;
   }
   else if(features->term_cells < limits->term_large_limit)
   {
      features->set_termcell_size = SpecMediumTerms;
   }
   else
   {
      features->set_termcell_size = SpecLargeTerms;
   }

   switch(features->max_fun_arity)
   {
   case 0:
         features->max_fun_ar_class = SpecArity0;
         break;
   case 1:
         features->max_fun_ar_class = SpecArity1;
         break;
   case 2:
         features->max_fun_ar_class = SpecArity2;
         break;
   default:
         features->max_fun_ar_class = SpecArity3Plus;
         break;
   }
   switch(features->avg_fun_arity)
   {
   case 0:
         features->avg_fun_ar_class = SpecArity0;
         break;
   case 1:
         features->avg_fun_ar_class = SpecArity1;
         break;
   case 2:
         features->avg_fun_ar_class = SpecArity2;
         break;
   default:
         features->avg_fun_ar_class = SpecArity3Plus;
         break;
   }
   features->ng_unit_axioms_part = features->unitaxioms?
      ((double)(features->unitaxioms-features->groundunitaxioms)/
       (double)(features->unitaxioms))
      :0.0;

   features->ground_positive_axioms_part = features->positiveaxioms?
      ((double)(features->groundpositiveaxioms)/
       (double)(features->positiveaxioms))
      :0.0;
   if(features->sum_fun_arity < limits->far_sum_medium_limit)
   {
      features->sum_fun_ar_class = SpecAritySumSmall;
   }
   else if(features->sum_fun_arity < limits->far_sum_large_limit)
   {
      features->sum_fun_ar_class = SpecAritySumMedium;
   }
   else
   {
      features->sum_fun_ar_class = SpecAritySumLarge;
   }

   if(features->clause_max_depth < limits->depth_medium_limit)
   {
      features->max_depth_class = SpecDepthShallow;
      /* printf("Shallow %ld %ld\n", features->clause_max_depth, limits->depth_medium_limit);*/
   }
   else if(features->clause_max_depth < limits->depth_deep_limit)
   {
      features->max_depth_class = SpecDepthMedium;
      /* printf("Medium %ld %ld\n", features->clause_max_depth, limits->depth_medium_limit);*/
   }
   else
   {
      features->max_depth_class = SpecDepthDeep;
      /* printf("Deep %ld %ld\n", features->clause_max_depth, limits->depth_medium_limit);*/
   }

   if(features->order < 2)
   {
      features->order_class = SpecFO;
   }
   else if(features->order == 2)
   {
      features->order_class = SpecSO;
   }
   else
   {
      assert(features->order >= 3);
      features->order_class = SpecHO;
   }

   if(features->goal_order < 2)
   {
      features->goal_order_class = SpecFO;
   }
   else if(features->goal_order == 2)
   {
      features->goal_order_class = SpecSO;
   }
   else
   {
      assert(features->goal_order >= 3);
      features->goal_order_class = SpecHO;
   }

   if(features->num_of_definitions < limits->num_of_defs_medium_limit)
   {
      features->defs_class = SpecFewDefs;
   }
   else if(features->num_of_definitions < limits->num_of_defs_large_limit)
   {
      features->defs_class = SpecMediumDefs;
   }
   else
   {
      features->defs_class = SpecManyDefs;
   }

   if(features->perc_of_form_defs < limits->perc_form_defs_medium_limit)
   {
      features->form_defs_class = SpecFewFormDefs;
   }
   else if(features->perc_of_form_defs < limits->perc_form_defs_large_limit)
   {
      features->form_defs_class = SpecMediumFormDefs;
   }
   else
   {
      features->form_defs_class = SpecManyFormDefs;
   }

   if(features->perc_of_appvar_lits < limits->perc_app_lits_medium_limit)
   {
      features->appvar_lits_class = SpecFewApplits;
   }
   else if(features->perc_of_appvar_lits < limits->perc_app_lits_large_limit)
   {
      features->appvar_lits_class = SpecMediumApplits;
   }
   else
   {
      features->appvar_lits_class = SpecManyApplits;
   }
}




/*-----------------------------------------------------------------------
//
// Function: SpecFeaturesPrint()
//
//   Print the feature vector.
//
// Global Variables: -
//
// Side Effects    : Outpur
//
/----------------------------------------------------------------------*/

void SpecFeaturesPrint(FILE* out, SpecFeature_p features)
{
   assert(features);

   fprintf(out,
           "( %3ld, %3ld, %3ld, %3ld, %3ld, %3ld, %3ld, %3ld, %3ld,"
           " %3ld, %3ld, %3ld, %3ld, %3ld, %3ld, %8.6f, %8.6f,"
           " %3d, %3d, %3d, %3ld, %3ld, %3d, %3d, %8.6f, %8.6f, %s, %s )",
           features->goals,
           features->axioms,
           features->clauses,
           features->literals,
           features->term_cells,
           features->unitgoals,
           features->unitaxioms,
           features->horngoals,
           features->hornaxioms,
           features->eq_clauses,
           features->peq_clauses,
           features->groundunitaxioms,
           features->groundgoals,
           features->groundpositiveaxioms,
           features->positiveaxioms,
           features->ng_unit_axioms_part,
           features->ground_positive_axioms_part,
           features->max_fun_arity,
           features->avg_fun_arity,
           features->sum_fun_arity,
           features->clause_max_depth,
           features->clause_avg_depth,
           features->order,
           features->num_of_definitions,
           features->perc_of_form_defs,
           features->perc_of_appvar_lits,
           BOOL2STR(features->quantifies_booleans),
           BOOL2STR(features->has_defined_choice)
      );
}


/*-----------------------------------------------------------------------
//
// Function: SpecFeaturesParse()
//
//   Parse the relevant (i.e. currently used and printed) parts of a
//   spec features cell from in into a caller-provided structure. Also
//   parse the type and extract the invariant parts from it.
//
// Global Variables: -
//
// Side Effects    : Input
//
/----------------------------------------------------------------------*/

void SpecFeaturesParse(Scanner_p in, SpecFeature_p features)
{
   char *class;

   AcceptInpTok(in, OpenBracket);
   features->goals                       = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->axioms                      = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->clauses                     = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->literals                    = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->term_cells                  = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->unitgoals                   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->unitaxioms                  = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->horngoals                   = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->hornaxioms                  = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->eq_clauses                  = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->peq_clauses                 = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->groundunitaxioms            = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->groundgoals                 = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->groundpositiveaxioms        = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->positiveaxioms              = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->ng_unit_axioms_part         = ParseFloat(in);
   AcceptInpTok(in, Comma);
   features->ground_positive_axioms_part = ParseFloat(in);
   AcceptInpTok(in, Comma);
   features->max_fun_arity               = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->avg_fun_arity               = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->sum_fun_arity               = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->clause_max_depth            = ParseInt(in);
   AcceptInpTok(in, Comma);
   features->clause_avg_depth            = ParseInt(in);
   AcceptInpTok(in, CloseBracket);
   AcceptInpTok(in, Colon);

   class = ParsePlainFilename(in);
   if(strlen(class) < 5)
   {
      Error("Insufficient class information in class name(s) (to short)", SYNTAX_ERROR);
   }
   switch(class[0])
   {
   case 'G':
         features->axiomtypes = SpecGeneral;
         break;
   case 'H':
         features->axiomtypes = SpecHorn;
         break;
   case 'U':
         features->axiomtypes = SpecUnit;
         break;
   default:
         Error("Insufficient class information in class name(s)", SYNTAX_ERROR);
         break;
   }
   switch(class[1])
   {
   case 'H':
         features->goaltypes = SpecHorn;
         break;
   case 'U':
         features->goaltypes = SpecUnit;
         break;
   default:
         Error("Insufficient class information in class name(s)", SYNTAX_ERROR);
         break;
   }
   switch(class[2])
   {
   case 'N':
         features->eq_content = SpecNoEq;
         break;
   case 'S':
         features->eq_content = SpecSomeEq;
         break;
   case 'P':
         features->eq_content = SpecPureEq;
         break;
   default:
         Error("Insufficient class information in class name(s)", SYNTAX_ERROR);
         break;
   }
   /* class[3] information is recovered from numeric features */
   switch(class[4])
   {
   case 'G':
         features->goals_are_ground = true;
         break;
   case 'N':
         features->goals_are_ground = false;
         break;
   default:
         Error("Insufficient class information in class name(s)", SYNTAX_ERROR);
         break;
   }


   FREE(class);
}


/*-----------------------------------------------------------------------
//
// Function: SpecTypeString()
//
//   Encode the type of the problem as a n-letter code.
//   1) Axioms are [U]nit, [H]orn, [General]
//   2) Goals  are [U]nit, [H]orn, [General]
//   3) [N]o equality, [S]ome equality, [P]ure equality
//   4) [F]ew, [S]ome, [M]any non-ground facts
//   5) [G]round goals or [N]on-ground goals
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/
#define GET_ENCODING(idx) (assert((idx) < (enc_len)), encoding[(idx)])
char* SpecTypeString(SpecFeature_p features, const char* mask)
{
   const char encoding[]="UHGNSPFSMFSMFSMFSMSML0123SMLSMDFSHFSMFSMFSM";
#ifndef NDEBUG
   const int enc_len = strlen(encoding);
#endif
   char       result[SPEC_STRING_MEM]; /* Big enough for the '\0'!!!*/
   int        i, limit;

   assert(features);
   assert(mask && (strlen(mask)>=13) && (strlen(mask)<=22));
   limit = strlen(mask);
   snprintf(result, 22, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
           problemType == PROBLEM_HO ? 'H' : 'F',
           GET_ENCODING(features->axiomtypes),
           GET_ENCODING(features->goaltypes),
           GET_ENCODING(features->eq_content),
           GET_ENCODING(features->ng_unit_content),
           features->goals_are_ground?'G':'N',
           GET_ENCODING(features->set_clause_size),
           GET_ENCODING(features->set_literal_size),
           GET_ENCODING(features->set_termcell_size),
           GET_ENCODING(features->ground_positive_content),
           GET_ENCODING(features->max_fun_ar_class),
           GET_ENCODING(features->avg_fun_ar_class),
           GET_ENCODING(features->sum_fun_ar_class),
           GET_ENCODING(features->max_depth_class),
           GET_ENCODING(features->order_class),
           GET_ENCODING(features->goal_order_class),
           GET_ENCODING(features->defs_class),
           GET_ENCODING(features->form_defs_class),
           GET_ENCODING(features->appvar_lits_class),
           features->quantifies_booleans?'B':'N',
           features->has_defined_choice?'C':'N');
   for(i=0; i<limit; i++)
   {
      if(mask[i]=='-')
      {
         result[i]= '-';
      }
   }
   return SecureStrndup(result, 21);
}


/*-----------------------------------------------------------------------
//
// Function: SpecTypePrint()
//
//   Print the string created by SpecTypeString
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void SpecTypePrint(FILE* out, SpecFeature_p features, char* mask)
{
   char* result = SpecTypeString(features, mask);
   fputs(result, out);
   FREE(result);
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetPrintPosUnits()
//
//   Print the positive unit clauses from set.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPrintPosUnits(FILE* out, ClauseSet_p set, bool
                            printinfo)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsDemodulator(handle))
      {
         ClauseLinePrint(out, handle, printinfo);
      }
   }
}

/*-----------------------------------------------------------------------
//
// Function: ClauseSetPrintNegUnits()
//
//   Print the negative unit clauses from set.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPrintNegUnits(FILE* out, ClauseSet_p set, bool
                            printinfo)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(ClauseIsUnit(handle)&&ClauseIsGoal(handle))
      {
         ClauseLinePrint(out, handle, printinfo);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetPrintNonUnits()
//
//   Print the non-unit clauses from set.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ClauseSetPrintNonUnits(FILE* out, ClauseSet_p set, bool
                            printinfo)
{
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
          handle->succ)
   {
      if(!ClauseIsUnit(handle))
      {
         ClauseLinePrint(out, handle, printinfo);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ProofStatePrintSelective()
//
//   Print parts of the proof state to the given stream. Descriptor
//   controls which parts.
//
// Global Variables: -
//
// Side Effects    : Output
//
/----------------------------------------------------------------------*/

void ProofStatePrintSelective(FILE* out, ProofState_p state, char*
                              descriptor, bool printinfo)
{
   char* current = descriptor;


   while(*current)
   {
      switch(*current)
      {
      case 't':
            if(problemType == PROBLEM_HO || !ClauseSetIsUntyped(state->axioms))
            {
               fprintf(out, COMCHAR" Type declarations:\n");
               SigPrintTypeDeclsTSTP(out, state->terms->sig);
            }
            break;
      case 'e':
            fprintf(out, COMCHAR" Processed positive unit clauses:\n");
            ClauseSetPrintPosUnits(out, state->processed_pos_rules,
                                   printinfo);
            ClauseSetPrintPosUnits(out, state->processed_pos_eqns,
                                   printinfo);
            fputc('\n', out);
            break;
      case 'i':
            fprintf(out, COMCHAR" Processed negative unit clauses:\n");
            ClauseSetPrintNegUnits(out, state->processed_neg_units,
                                   printinfo);
            fputc('\n', out);
            break;
      case 'g':
            fprintf(out, COMCHAR" Processed non-unit clauses:\n");
            ClauseSetPrintNonUnits(out, state->processed_non_units,
                                   printinfo);
            fputc('\n', out);
            break;
      case 'E':
            fprintf(out, COMCHAR" Unprocessed positive unit clauses:\n");
            ClauseSetPrintPosUnits(out, state->unprocessed,
                                   printinfo);
            fputc('\n', out);
            break;
      case 'I':
            fprintf(out, COMCHAR" Unprocessed negative unit clauses:\n");
            ClauseSetPrintNegUnits(out, state->unprocessed,
                                   printinfo);
            fputc('\n', out);
            break;
      case 'G':
            fprintf(out, COMCHAR" Unprocessed non-unit clauses:\n");
            ClauseSetPrintNonUnits(out, state->unprocessed,
                                   printinfo);
            fputc('\n', out);
            break;
      case 'a':
            if(ClauseSetIsEquational(state->axioms))
            {
               fprintf(out, COMCHAR" Equality axioms:\n");
               EqAxiomsPrint(out, state->signature, true);
            }
            else
            {
               fprintf(out, COMCHAR" No equality axioms required.\n");
            }

            break;
      case 'A':
            if(ClauseSetIsEquational(state->axioms))
            {
               fprintf(out, COMCHAR" Equality axioms:\n");
               EqAxiomsPrint(out, state->signature, false);
            }
            else
            {
               fprintf(out, COMCHAR" No equality axioms required.\n");
            }
            break;
      default:
            assert(false && "Illegal character in descriptor string");
            break;
      }
      current++;
   }
}


/*-----------------------------------------------------------------------
//
// Function: CreateDefaultSpecLimits()
//
//   Return a SpecLimits cell initialized with the default limits for
//   Auto-Mode problem classification.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/
SpecLimits_p CreateDefaultSpecLimits(void)
{
   SpecLimits_p limits =  SpecLimitsAlloc();

   limits->ax_some_limit        = 1000;
   limits->ax_many_limit        = 100000;
   limits->lit_some_limit       = 400;
   limits->lit_many_limit       = 4000;
   limits->term_medium_limit    = 200;
   limits->term_large_limit     = 1500;
   limits->far_sum_medium_limit = 4;
   limits->far_sum_large_limit  = 29;
   limits->depth_medium_limit   = 4;
   limits->depth_deep_limit     = 7;
   limits->gpc_absolute         = true;
   limits->gpc_few_limit        = 2;
   limits->gpc_many_limit       = 5;
   limits->ngu_absolute         = true;
   limits->ngu_few_limit        = 1;
   limits->ngu_many_limit       = 3;

   limits->order_medium_limit   = ORDER_MEDIUM_DEFAULT;
   limits->order_large_limit    = ORDER_LARGE_DEFAULT;
   limits->num_of_defs_medium_limit = DEFS_MEDIUM_DEFAULT;
   limits->num_of_defs_large_limit = DEFS_LARGE_DEFAULT;
   limits->perc_form_defs_medium_limit = DEFS_PERC_MEDIUM_DEFAULT;
   limits->perc_form_defs_large_limit = DEFS_PERC_LARGE_DEFAULT;
   limits->num_of_lams_medium_limit = NUM_LAMS_MEDIUM_DEFAULT;
   limits->num_of_lams_large_limit = NUM_LAMS_LARGE_DEFAULT;
   limits->perc_app_lits_medium_limit = PERC_APPLIT_MEDIUM_DEFAULT;
   limits->perc_app_lits_large_limit = PERC_APPLIT_LARGE_DEFAULT;

   return limits;
}


/*-----------------------------------------------------------------------
//
// Function: ClauseSetComputeHOFeatures()
//
//  Fill in the HO statistics such as: are there non-FO features  of the
//  problem, what is the maximal term order in the problem, does the
//  problem quantify booleans and does it have defined choice clauses.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

#define IS_NON_FO_TERM(t) ( TermIsNonFOPattern(t) || TermHasLambdaSubterm(t) || TermHasDBSubterm(t) )

void ClauseSetComputeHOFeatures(ClauseSet_p set, Sig_p sig,
                                bool* has_ho_features,
                                int* order,
                                bool* quantifies_bools,
                                bool* has_defined_choice,
                                double* perc_app_var_lits)
{
   Clause_p handle;
   bool is_fo = true;
   bool var_has_bools = false;
   bool has_choice = false;
   int av_lits = 0;

   int ord = 0;
   for(FunCode i = sig->internal_symbols+1; i<=sig->f_count; i++)
   {
      ord = MAX(ord, TypeGetOrder(SigGetType(sig, i)));
   }

   for(handle = set->anchor->succ; handle!=set->anchor;
       handle = handle->succ)
   {
      PTree_p vars = NULL;
      ClauseCollectVariables(handle, &vars);
      PStack_p iter = PTreeTraverseInit(vars);
      PTree_p node;
      while((node = PTreeTraverseNext(iter)))
      {
         Type_p ty = ((Term_p)node->key)->type;
         ord = MAX(ord, VAR_ORDER(ty));
         var_has_bools = var_has_bools || TypeHasBool(ty);
      }
      PTreeTraverseExit(iter);
      PTreeFree(vars);

      bool has_app_var = false;
      for(Eqn_p eqn = handle->literals; eqn; eqn = eqn->next)
      {
         is_fo = is_fo && !IS_NON_FO_TERM(eqn->lterm)
                       && !IS_NON_FO_TERM(eqn->rterm);
         has_app_var = has_app_var || TermIsAppliedFreeVar(eqn->lterm)
                                   || TermIsAppliedFreeVar(eqn->rterm);
      }

      has_choice = has_choice || ClauseRecognizeChoice(NULL, handle);
      av_lits += has_app_var ? 1 : 0;
   }

   *order = ord;
   *has_ho_features = !is_fo;
   *quantifies_bools = var_has_bools;
   *has_defined_choice = has_choice;
   *perc_app_var_lits =
      ClauseSetCardinality(set) ? ((double)av_lits / ClauseSetCardinality(set)) : 0.0 ;
}

/*-----------------------------------------------------------------------
//
// Function: SpecLimitsPrint()
//
//  Fill in the HO statistics such as: are there non-FO features  of the
//  problem, what is the maximal term order in the problem, does the
//  problem quantify booleans and does it have defined choice clauses.
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void SpecLimitsPrint(FILE* out, SpecLimits_p limits)
{
   fprintf(out, "[ %d | %g | %g | %d | %g | %g |"
                   " %ld | %ld | %ld | %ld | %ld | %ld | %ld | %ld | %ld | %ld | "
                   " %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | %d | "
                   " %g | %g | %g | %g ]\n",
   limits->ngu_absolute,
   limits->ngu_few_limit,
   limits->ngu_many_limit,
   limits->gpc_absolute,
   limits->gpc_few_limit,
   limits->gpc_many_limit,
   limits->ax_some_limit,
   limits->ax_many_limit,
   limits->lit_some_limit,
   limits->lit_many_limit,
   limits->term_medium_limit,
   limits->term_large_limit,
   limits->far_sum_medium_limit,
   limits->far_sum_large_limit,
   limits->depth_medium_limit,
   limits->depth_deep_limit,
   limits->symbols_medium_limit,
   limits->symbols_large_limit,
   limits->predc_medium_limit,
   limits->predc_large_limit,
   limits->pred_medium_limit,
   limits->pred_large_limit,
   limits->func_medium_limit,
   limits->func_large_limit,
   limits->fun_medium_limit,
   limits->fun_large_limit,
   limits->order_medium_limit,
   limits->order_large_limit,
   limits->num_of_lams_medium_limit,
   limits->num_of_lams_large_limit,
   limits->num_of_defs_medium_limit,
   limits->num_of_defs_large_limit,
   limits->perc_form_defs_medium_limit,
   limits->perc_form_defs_large_limit,
   limits->perc_app_lits_medium_limit,
   limits->perc_app_lits_large_limit);
}

/*-----------------------------------------------------------------------
//
// Function: ClausifyAndClassifyWTimeout()
//
//   Run the defaultclausification and get the corresponding classification
//   string. If last three arguments are non-NULL, the full classification string
//   with computed features will be output to stdout.
//
// Global Variables: Plenty, most simple flags used read-only
//
// Side Effects    : Does everything...
//
/----------------------------------------------------------------------*/

void ClausifyAndClassifyWTimeout(ProofState_p state, int timeout,
                                 char* mask,
                                 char class[SPEC_STRING_MEM])
{
   const int DEFAULT_MINISCOPE = 1048576;
   const int DEFAULT_FORMULA_DEF_LIMIT = 24;
   const bool DEFAULT_LIFT_LAMS = false;
   const bool DEFAULT_LAM_TO_FORALL = true;
   const bool DEFAULT_UNFOLD_ONLY_FORM = true;
   const bool DEFAULT_UNROLL_FOOL = true;

   int fds[2];
   if(pipe(fds) == -1)
   {
      perror("pipe failed");
      exit(1);
   }

   SpecFeatureCell features;
   SpecLimits_p limits = CreateDefaultSpecLimits();

   pid_t pid = fork();
   if (pid == -1)
   {
      perror("fork failed");
      exit(1);
   }
   else if (pid == 0)
   {
      // child
      close(fds[0]);
      if(SetSoftRlimit(RLIMIT_CPU, timeout) != RLimSuccess)
      {
         fprintf(stderr, "softrlimit call failed.\n");
         exit(-1);
      }
      FormulaSetPreprocConjectures(state->f_axioms, state->f_ax_archive, false, false);
      FormulaSetCNF2(state->f_axioms, state->f_ax_archive,
                     state->axioms, state->terms,
                     state->freshvars,
                     DEFAULT_MINISCOPE, DEFAULT_FORMULA_DEF_LIMIT,
                     DEFAULT_LIFT_LAMS, DEFAULT_LAM_TO_FORALL,
                     DEFAULT_UNFOLD_ONLY_FORM, DEFAULT_UNROLL_FOOL);
      SpecFeaturesCompute(&features, state->axioms, state->f_axioms,
                          state->f_ax_archive, state->terms);
      SpecFeaturesAddEval(&features, limits);
      if(write(fds[1], SpecTypeString(&features, mask), SPEC_STRING_MEM) == -1)
      {
         perror("could not write");
      }
      exit(0);
   }
   else
   {
      // parent
      close(fds[1]);
      int nbytes = read(fds[0], class, SPEC_STRING_MEM);
      if(nbytes < SPEC_STRING_MEM)
      {
         memset(class, '-', SPEC_STRING_MEM-1);
         class[SPEC_STRING_MEM-1]='\0';
      }
      SpecLimitsCellFree(limits);
      waitpid(pid, NULL, 0); // collect exit status
   }
}
/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
