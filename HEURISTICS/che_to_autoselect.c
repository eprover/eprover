/*-----------------------------------------------------------------------

File  : che_to_autoselect.c

Author: Stephan Schulz

Contents

  Functions for creating term orderings.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Fri Jan  1 16:06:31 MET 1999
    New

-----------------------------------------------------------------------*/

#include "che_to_autoselect.h"



/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#define TO_ORDERING_INTERNAL

/*-----------------------------------------------------------------------
//
// Function: init_oparms()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void init_oparms(OrderParms_p oparms)
{
   oparms->ordertype       = KBO6;
   oparms->to_const_weight = WConstNoSpecialWeight;
   oparms->to_weight_gen   = WSelectMaximal;
   oparms->to_prec_gen     = PUnaryFirst;
   oparms->no_lit_cmp      = false;

}

/*-----------------------------------------------------------------------
//
// Function: print_oparms()
//
//
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

void print_oparms(OrderParms_p oparms)
{

   if(OutputLevel)
   {
      fprintf(GlobalOut, "# Auto-mode selected ordering type %s\n",
              TONames[oparms->ordertype]);
      fprintf(GlobalOut, "# Auto-mode selected ordering precedence scheme <%s>\n",
              TOGetPrecGenName(oparms->to_prec_gen));
      if(oparms->ordertype == KBO || oparms->ordertype == KBO6)
      {
         fprintf(GlobalOut, "# Auto-mode selected weight ordering scheme <%s>\n",
                 TOGetWeightGenName(oparms->to_weight_gen));
      }
      fputs("#\n", GlobalOut);
   }
}


/*-----------------------------------------------------------------------
//
// Function: generate_auto_ordering()
//
//   Generate a term ordering suitable to the problem in state.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may call SpecFeaturesAddEval()
//                   and change cheap parts of specs.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_auto_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# Auto-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO


/*-----------------------------------------------------------------------
//
// Function: generate_autocasc_ordering()
//
//   Generate a term ordering suitable to the problem in state. This
//   is the CASC-20 auto mode
//
// Global Variables: -
//
// Side Effects    : Memory operations, may call SpecFeaturesAddEval()
//                   and change cheap parts of specs.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO_CASC

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autocasc_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = SpecLimitsAlloc();

   limits->ax_some_limit        = 46;
   limits->ax_many_limit        = 205;
   limits->lit_some_limit       = 212;
   limits->lit_many_limit       = 620;
   limits->term_medium_limit    = 163;
   limits->term_large_limit     = 2270;
   limits->far_sum_medium_limit = 5;
   limits->far_sum_large_limit  = 24;
   limits->depth_medium_limit   = 0;
   limits->depth_deep_limit     = 6;
   limits->gpc_absolute         = true;
   limits->gpc_few_limit        = 1;
   limits->gpc_many_limit       = 3;
   limits->ngu_absolute         = true;
   limits->ngu_few_limit        = 1;
   limits->ngu_many_limit       = 3;

   oparms.ordertype       = KBO6;
   oparms.to_const_weight = WConstNoSpecialWeight;
   oparms.to_weight_gen   = WSelectMaximal;
   oparms.to_prec_gen     = PUnaryFirst;
   oparms.no_lit_cmp      = false;
   OUTPRINT(1, "\n# Auto-Ordering is analysing problem.\n");

#include "che_auto_cases.c"

   if(OutputLevel)
   {
      fprintf(GlobalOut, "# Auto-mode selected ordering type %s\n",
              TONames[oparms.ordertype]);
      fprintf(GlobalOut, "# Auto-mode selected ordering precedence scheme <%s>\n",
              TOGetPrecGenName(oparms.to_prec_gen));
      if(oparms.ordertype == KBO || oparms.ordertype == KBO6)
      {
         fprintf(GlobalOut, "# Auto-mode selected weight ordering scheme <%s>\n",
                 TOGetWeightGenName(oparms.to_weight_gen));
      }
      fputs("#\n", GlobalOut);
   }
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_CASC



/*-----------------------------------------------------------------------
//
// Function: generate_autodev_ordering()
//
//   Generate a term ordering suitable to the problem in state.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may call SpecFeaturesAddEval()
//                   and change cheap parts of specs.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO_DEV

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autodev_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   oparms.ordertype       = KBO6;
   oparms.to_const_weight = WConstNoSpecialWeight;
   oparms.to_weight_gen   = WSelectMaximal;
   oparms.to_prec_gen     = PUnaryFirst;
   oparms.no_lit_cmp      = false;

   OUTPRINT(1, "\n# Auto-Ordering is analysing problem.\n");
#include "che_auto_cases.c"

   if(OutputLevel)
   {
      fprintf(GlobalOut, "# Auto-mode (Dev) selected ordering type %s\n",
              TONames[oparms.ordertype]);
      fprintf(GlobalOut, "# Auto-mode (Dev) selected ordering precedence scheme <%s>\n",
              TOGetPrecGenName(oparms.to_prec_gen));
      if(oparms.ordertype == KBO || oparms.ordertype == KBO6)
      {
         fprintf(GlobalOut, "# Auto-mode (Dev) selected weight ordering scheme <%s>\n",
                 TOGetWeightGenName(oparms.to_weight_gen));
      }
      fputs("#\n", GlobalOut);
   }
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_DEV


/*-----------------------------------------------------------------------
//
// Function: generate_autosched0_ordering()
// Function: generate_autosched1_ordering()
// Function: generate_autosched2_ordering()
// Function: generate_autosched3_ordering()
// Function: generate_autosched4_ordering()
// Function: generate_autosched5_ordering()
// Function: generate_autosched6_ordering()
// Function: generate_autosched7_ordering()
//
//   Generate term orderings according to the selected auto-schedule
//   mode.
//
// Global Variables: -
//
// Side Effects    : Memory operations, may call SpecFeaturesAddEval()
//                   and change cheap parts of specs.
//
/----------------------------------------------------------------------*/

#define CHE_HEURISTICS_AUTO_SCHED0

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched0_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched0-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED0

#define CHE_HEURISTICS_AUTO_SCHED1

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched1_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched1-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED1

#define CHE_HEURISTICS_AUTO_SCHED2

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched2_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched2-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED2

#define CHE_HEURISTICS_AUTO_SCHED3

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched3_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched3-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED3

#define CHE_HEURISTICS_AUTO_SCHED4

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched4_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched4-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED4

#define CHE_HEURISTICS_AUTO_SCHED5

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched5_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched5-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED5

#define CHE_HEURISTICS_AUTO_SCHED6

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched6_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched6-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED6

#define CHE_HEURISTICS_AUTO_SCHED7

GCC_DIAGNOSTIC_PUSH
#ifndef COMPILE_HEURISTICS_OPTIMIZED
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((optnone))
__attribute__((optimize(0)))
#endif
OCB_p generate_autosched7_ordering(ProofState_p state, SpecFeature_p spec)
{
   OrderParmsCell  oparms;
   SpecLimits_p    limits = CreateDefaultSpecLimits();

   init_oparms(&oparms);
   OUTPRINT(1, "\n# AutoSched7-Ordering is analysing problem.\n");
#include "che_auto_cases.c"
   print_oparms(&oparms);
   SpecLimitsCellFree(limits);
   return TOCreateOrdering(state, &oparms, NULL, NULL);
}
GCC_DIAGNOSTIC_POP
#undef CHE_HEURISTICS_AUTO_SCHED67




/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: OrderParmsInitialize()
//
//   Given a HeuristicParmsCell and an OrderParmsCell, initialize the
//   OrderParmsCell with values from the HeuristicParmsCell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void OrderParmsInitialize(HeuristicParms_p master, OrderParms_p slave)
{
   slave->ordertype       = master->ordertype;
   slave->to_weight_gen   = master->to_weight_gen;
   slave->to_prec_gen     = master->to_prec_gen;
   slave->to_const_weight = master->to_const_weight;
   slave->no_lit_cmp      = master->no_lit_cmp;
}


/*-----------------------------------------------------------------------
//
// Function: HeuristicParmsUpdate()
//
//   Given a HeuristicParmsCell and an OrderParmsCell, update the
//   HeuristicParmsCell with values from the OrderParmsCell.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void HeuristicParmsUpdate(OrderParms_p master, HeuristicParms_p slave)
{
   slave->ordertype       = master->ordertype;
   slave->to_weight_gen   = master->to_weight_gen;
   slave->to_prec_gen     = master->to_prec_gen;
   slave->to_const_weight = master->to_const_weight;
}


/*-----------------------------------------------------------------------
//
// Function: OrderEvaluate()
//
//   Given an OCB, evaluate the resulting ordering on the axioms of
//   state. Low is good.
//
// Global Variables: -
//
// Side Effects    : Orients the axioms (should not really matter)
//
/----------------------------------------------------------------------*/

#define KBO_BONUS 1
#define MAX_TERM_PENALTY 2
#define MAX_LITERAL_PENALTY 1
#define UNORIENT_LITERAL_PENALTY 1

double OrderEvaluate(OCB_p ocb, ProofState_p state, HeuristicParms_p
                     parms)
{
   double res = 0;

   ClauseSetMarkMaximalTerms(ocb, state->axioms);
   res+= (ClauseSetCountMaximalTerms(state->axioms)
          *MAX_TERM_PENALTY);
   res+= (ClauseSetCountMaximalLiterals(state->axioms)
          *MAX_LITERAL_PENALTY);
   res+= (ClauseSetCountUnorientableLiterals(state->axioms)
          *UNORIENT_LITERAL_PENALTY);
   if(ocb->type == KBO)
   {
      res = res*KBO_BONUS;
   }
   /* printf("OrderEvaluate()=%f\n", res); */
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: OrderNextType()
//
//   In an implicit ordering on TermOrdering, set ordering->ordertype
//   to the next value (if it exists) and return true. Set it to
//   NoOrdering and return false otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OrderNextType(OrderParms_p ordering)
{
   bool res = true;

   assert(ordering);

   switch(ordering->ordertype)
   {
   case NoOrdering:
         ordering->ordertype = KBO;
         break;
   case KBO:
         ordering->ordertype = LPO;
         break;
   case LPO:
         ordering->ordertype = NoOrdering;
         res = false;
         break;
   default:
         res = false;
         assert(false&& "Unexpected ordertype!");
         break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: OrderNextWeightGen()
//
//   Set ordering->to_weight_gen to the next value if it exists, to
//   WNoMethod if not. Return true if next value existed, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OrderNextWeightGen(OrderParms_p ordering)
{
   assert(ordering);
   assert(ordering->to_weight_gen <= WMaxMethod);

   if(ordering->to_weight_gen == WMaxMethod)
   {
      ordering->to_weight_gen = WNoMethod;
      return false;
   }
   ordering->to_weight_gen++;
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: OrderNextPrecGen()
//
//   Set ordering->to_prec_gen to the next value if it exists, to
//   PNoMethod if not. Return true if next value existed, false
//   otherwise.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OrderNextPrecGen(OrderParms_p ordering)
{
   assert(ordering);
   assert(ordering->to_prec_gen <= PMaxMethod);

   if(ordering->to_prec_gen == PMaxMethod)
   {
      ordering->to_prec_gen = PNoMethod;
      return false;
   }
   ordering->to_prec_gen++;
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: OrderNextConstWeight()
//
//   Set ordering->to_const_weight to the next value <=
//   MAX_CONST_WEIGHT or to WConstNoSpecialWeight if already at
//   MAX_CONST_WEIGHT. Return true in this case. Otherwise, set
//   to_const_weight to WConstNoWeight and return false.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

#define MAX_CONST_WEIGHT 2

bool OrderNextConstWeight(OrderParms_p ordering)
{
   assert(ordering);
   assert((ordering->to_const_weight == WConstNoSpecialWeight)||
          (ordering->to_const_weight == WConstNoWeight)||
          (ordering->to_const_weight > 0));

   if(ordering->to_const_weight == WConstNoSpecialWeight)
   {
      ordering->to_const_weight = WConstNoWeight;
      return false;
   }
   else if(ordering->to_const_weight == MAX_CONST_WEIGHT)
   {
      ordering->to_const_weight = WConstNoSpecialWeight;
      return true;
   }
   ordering->to_const_weight++;
   return true;
}


/*-----------------------------------------------------------------------
//
// Function: OrderNextOrdering()
//
//   Set ordering to the next possible ordering by alternating those
//   of the 4 parameters that are indeterminate in mask (NoOrdering,
//   PNoMethod, WNoMethod, WConstNoWeight). Return true if
//   successful, false otherwise (in which case ordering will have
//   cycled to the first possible combination, but don't count on
//   this, it is an artifact of this particular implementations).
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

bool OrderNextOrdering(OrderParms_p ordering, OrderParms_p mask)
{
   if(ordering->ordertype == KBO)
   {
      if(mask->to_const_weight == WConstNoWeight)
      {
         if(OrderNextConstWeight(ordering))
         {
            return true;
         }
         OrderNextConstWeight(ordering);
      }
      if(mask->to_prec_gen == PNoMethod)
      {
         if(OrderNextPrecGen(ordering))
         {
            return true;
         }
         OrderNextPrecGen(ordering);
      }
   }
   if(mask->to_weight_gen == WNoMethod)
   {
      if(OrderNextWeightGen(ordering))
      {
         return true;
      }
      OrderNextWeightGen(ordering);
   }
   if(mask->ordertype == NoOrdering)
   {
      if(OrderNextType(ordering))
      {
         return true;
      }
      OrderNextType(ordering);
   }
   return false;
}

/*-----------------------------------------------------------------------
//
// Function: OrderFindOptimal()
//
//   Iterate through all orderings matching mask (see previous
//   function) and find the optimal one. Return a corresponding OCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations, whatever eval_fun does.
//
/----------------------------------------------------------------------*/

OCB_p OrderFindOptimal(OrderParms_p mask, OrderEvaluationFun eval_fun,
                       ProofState_p state, HeuristicParms_p parms)
{
   OrderParmsCell local, store;
   OCB_p          best_ocb, tmp_ocb;
   double         best_eval, tmp_eval;

   VERBOUT("Starting search for optimal term ordering.\n");
   local.ordertype =
      (mask->ordertype==NoOrdering)?KBO:mask->ordertype;
   local.to_weight_gen =
      (mask->to_weight_gen==WNoMethod)?WMinMethod:mask->to_weight_gen;
   local.to_prec_gen =
      (mask->to_prec_gen==PNoMethod)?PMinMethod:mask->to_prec_gen;
   local.to_const_weight =
      (mask->to_const_weight==WConstNoWeight)?1:mask->to_const_weight;

   store = local;
   best_ocb  = TOCreateOrdering(state, &local,NULL, NULL);
   best_eval = eval_fun(best_ocb, state, parms);

   while(OrderNextOrdering(&local, mask))
   {
      tmp_ocb  = TOCreateOrdering(state, &local, NULL, NULL);
      tmp_eval = eval_fun(tmp_ocb, state, parms);
      if(tmp_eval < best_eval)
      {
         OCBFree(best_ocb);
         best_ocb = tmp_ocb;
         best_eval = tmp_eval;
         store = local;
      }
      else
      {
         OCBFree(tmp_ocb);
      }
   }
   VERBOSE(fprintf(stderr, "%s: Term Ordering found: (%s, %s, %s, %ld)\n",
                   ProgName,
                   TONames[store.ordertype],
                   TOGetPrecGenName(store.to_prec_gen),
                   TOGetWeightGenName(store.to_weight_gen),
                   store.to_const_weight););
   return best_ocb;
}



/*-----------------------------------------------------------------------
//
// Function: TOSelectOrdering()
//
//   Given a proof state, select a (hopefully suitable) ordering for
//   it and return the corresponding OCB.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

OCB_p TOSelectOrdering(ProofState_p state, HeuristicParms_p params,
                       SpecFeature_p specs)
{
   OrderParmsCell tmp;
   OCB_p          result;

   OrderParmsInitialize(params, &tmp);
   if(tmp.ordertype == OPTIMIZE_AX)
   {
      OrderParmsCell local;
      OrderParmsInitialize(params, &local);

      result = OrderFindOptimal(&local, OrderEvaluate, state, params);
   }
   else if(tmp.ordertype == AUTO)
   {
      result = generate_auto_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOCASC)
   {
      result = generate_autocasc_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTODEV)
   {
      result = generate_autodev_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED0)
   {
      result = generate_autosched0_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED1)
   {
      result = generate_autosched1_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED2)
   {
      result = generate_autosched2_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED3)
   {
      result = generate_autosched3_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED4)
   {
      result = generate_autosched4_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED5)
   {
      result = generate_autosched5_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED6)
   {
      result = generate_autosched6_ordering(state, specs);
   }
   else if(tmp.ordertype == AUTOSCHED7)
   {
      result = generate_autosched7_ordering(state, specs);
   }
   else
   {
      if(tmp.ordertype == NoOrdering)
      {
         tmp.ordertype = KBO;
      }
      if(tmp.to_const_weight==WConstNoWeight)
      {
         tmp.to_const_weight = WConstNoSpecialWeight;
      }
      result = TOCreateOrdering(state, &tmp, params->to_pre_prec,
                                params->to_pre_weights);
   }
   return result;
}


/*-----------------------------------------------------------------------
//
// Function: TOCreateOrdering()
//
//   Given a proof state and a fully specified OrderParamCell, create
//   the ordering.
//
// Global Variables: -
//
// Side Effects    : Memory operations
//
/----------------------------------------------------------------------*/

OCB_p  TOCreateOrdering(ProofState_p state, OrderParms_p params,
                        char* pre_precedence, char* pre_weights)
{
   OCB_p handle;
   bool prec_by_weight = pre_precedence?false:true;

   /* printf("TOCreateOrdering(%d, %d, %d, %ld)\n", params->ordertype,
      params->to_weight_gen, params->to_prec_gen,
      params->to_const_weight); */

   switch(params->ordertype)
   {
   case LPO:
         handle = OCBAlloc(LPO, prec_by_weight, state->signature);
         TOGeneratePrecedence(handle, state->axioms, pre_precedence,
                              params->to_prec_gen);
         break;
   case LPOCopy:
         handle = OCBAlloc(LPOCopy, prec_by_weight, state->signature);
         TOGeneratePrecedence(handle, state->axioms, pre_precedence,
                              params->to_prec_gen);
         break;
   case LPO4:
         handle = OCBAlloc(LPO4, prec_by_weight, state->signature);
         TOGeneratePrecedence(handle, state->axioms, pre_precedence,
                              params->to_prec_gen);
         break;
   case LPO4Copy:
         handle = OCBAlloc(LPO4Copy, prec_by_weight, state->signature);
         TOGeneratePrecedence(handle, state->axioms, pre_precedence,
                              params->to_prec_gen);
         break;
   case KBO:
         handle = OCBAlloc(KBO, prec_by_weight, state->signature);
         TOGeneratePrecedence(handle, state->axioms, pre_precedence,
                              params->to_prec_gen);
         TOGenerateWeights(handle, state->axioms,
                           pre_weights,
                           params->to_weight_gen,
                           params->to_const_weight);
         break;
   case KBO6:
         handle = OCBAlloc(KBO6, prec_by_weight, state->signature);
         TOGeneratePrecedence(handle, state->axioms, pre_precedence,
                              params->to_prec_gen);
         TOGenerateWeights(handle, state->axioms,
                           pre_weights,
                           params->to_weight_gen,
                           params->to_const_weight);
         break;
   case RPO:
         assert(false && "RPO not yet implemented!");
         handle = NULL;
         break;
   default:
         printf("What? %d\n",params->ordertype);
         assert(false&&"Incompletely specified OrderParamsCell");
         handle = NULL;
         break;
   }
   handle->no_lit_cmp = params->no_lit_cmp;
   return handle;
}

#undef TO_ORDERING_INTERNAL


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
