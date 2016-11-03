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

Changes

<1> Tue Sep 29 02:50:34 MET DST 1998
    New

-----------------------------------------------------------------------*/

#include "che_clausesetfeatures.h"


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
   handle->ax_some_limit          = AX_SOME_DEFAULT       ;
   handle->ax_many_limit          = AX_MANY_DEFAULT       ;
   handle->lit_some_limit         = LIT_SOME_DEFAULT      ;
   handle->lit_many_limit         = LIT_MANY_DEFAULT      ;
   handle->term_medium_limit      = TERM_MED_DEFAULT      ;
   handle->term_large_limit       = TERM_LARGE_DEFAULT    ;
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
// Function: ClauseSetCountTPTPRangeRestricted()
//
//   Count number of positive ground clauses.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long ClauseSetCountTPTPRangeRestricted(ClauseSet_p set)
{
   long     res = 0;
   Clause_p handle;

   for(handle = set->anchor->succ; handle!=set->anchor; handle =
     handle->succ)
   {
      if(ClauseIsTPTPRangeRestricted(handle))
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
      res += ClauseWeight(handle, 1, 1, 1, 1, 1, false);
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
          Sig_p sig)
{
   long tmp, count;

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

   if(features->clauses < limits->ax_some_limit)
   {
      features->set_clause_size = SpecFewAxioms;
   }
   else if(features->clauses < limits->ax_many_limit)
   {
      features->set_clause_size = SpecSomeAxioms;
   }
   else
   {
      features->set_clause_size = SpecManyAxioms;
   }

   if(features->literals < limits->lit_some_limit)
   {
      features->set_literal_size = SpecFewLiterals;
   }
   else if(features->literals < limits->lit_many_limit)
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
           " %3d, %3d, %3d, %3ld, %3ld )",
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
           features->clause_avg_depth
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
// Function: SpecTypePrint()
//
//   Print the type of the problem as a n-letter code.
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

void SpecTypePrint(FILE* out, SpecFeature_p features, char* mask)
{
   const char encoding[]="UHGNSPFSMFSMFSMFSMSML0123SMLSMD";
   char       result[14]; /* Big enough for the '\0'!!!*/
   int        i, limit;

   assert(features);
   assert(mask && (strlen(mask)==13));
   limit = strlen(mask);

   sprintf(result, "%c%c%c%c%c%c%c%c%c%c%c%c%c",
      encoding[features->axiomtypes],
      encoding[features->goaltypes],
      encoding[features->eq_content],
      encoding[features->ng_unit_content],
      features->goals_are_ground?'G':'N',
      encoding[features->set_clause_size],
      encoding[features->set_literal_size],
      encoding[features->set_termcell_size],
      encoding[features->ground_positive_content],
      encoding[features->max_fun_ar_class],
      encoding[features->avg_fun_ar_class],
      encoding[features->sum_fun_ar_class],
           encoding[features->max_depth_class]);
   for(i=0; i<limit; i++)
   {
      if(mask[i]=='-')
      {
    result[i]= '-';
      }
   }
   fputs(result, out);
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
      case 'e':
       fprintf(out, "# Processed positive unit clauses:\n");
       ClauseSetPrintPosUnits(out, state->processed_pos_rules,
               printinfo);
       ClauseSetPrintPosUnits(out, state->processed_pos_eqns,
               printinfo);
       fputc('\n', out);
       break;
      case 'i':
       fprintf(out, "# Processed negative unit clauses:\n");
       ClauseSetPrintNegUnits(out, state->processed_neg_units,
               printinfo);
       fputc('\n', out);
       break;
      case 'g':
       fprintf(out, "# Processed non-unit clauses:\n");
       ClauseSetPrintNonUnits(out, state->processed_non_units,
               printinfo);
       fputc('\n', out);
       break;
      case 'E':
       fprintf(out, "# Unprocessed positive unit clauses:\n");
       ClauseSetPrintPosUnits(out, state->unprocessed,
               printinfo);
       fputc('\n', out);
       break;
      case 'I':
       fprintf(out, "# Unprocessed negative unit clauses:\n");
       ClauseSetPrintNegUnits(out, state->unprocessed,
               printinfo);
       fputc('\n', out);
       break;
      case 'G':
       fprintf(out, "# Unprocessed non-unit clauses:\n");
       ClauseSetPrintNonUnits(out, state->unprocessed,
               printinfo);
       fputc('\n', out);
       break;
      case 'a':
       if(ClauseSetIsEquational(state->axioms))
       {
          fprintf(out, "# Equality axioms:\n");
          EqAxiomsPrint(out, state->signature, true);
       }
       else
       {
          fprintf(out, "# No equality axioms required.\n");
       }

       break;
      case 'A':
       if(ClauseSetIsEquational(state->axioms))
       {
          fprintf(out, "# Equality axioms:\n");
          EqAxiomsPrint(out, state->signature, false);
       }
       else
       {
          fprintf(out, "# No equality axioms required.\n");
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

   return limits;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


