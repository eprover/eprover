/*-----------------------------------------------------------------------

File  : pcl_lemmas.c

Author: Stephan Schulz

Contents

  Data structures and functions for the detection of lemmas in PCL2
  protocol.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jun 16 13:06:20 CEST 2003
    New

-----------------------------------------------------------------------*/

#include "pcl_lemmas.h"



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
// Function: LemmaParamAlloc()
//
//   Allocate an initialized parameter block for the lemma detection
//   algorithm.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

LemmaParam_p LemmaParamAlloc(void)
{
   LemmaParam_p handle = LemmaParamCellAlloc();

   handle->tree_base_weight   = LEMMA_TREE_BASE_W;
   handle->act_pm_w           = LEMMA_ACT_PM_W;
   handle->o_gen_w            = LEMMA_O_GEN_W;
   handle->act_simpl_w        = LEMMA_ACT_SIMPL_W;
   handle->pas_simpl_w        = LEMMA_PAS_SIMPL_W;
   handle->proof_tree_w       = LEMMA_PROOF_TREE_W;
   handle->proof_dag_w        = LEMMA_PROOF_DAG_W;
   handle->size_base_weight   = LEMMA_SIZE_BASE_W;
   handle->horn_bonus         = LEMMA_HORN_BONUS_W;

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: InferenceWeightsAlloc()
//
//   Allocate an inference weight parameter data structure,
//   initialized with default values.
//
// Global Variables: -
//
// Side Effects    : Memory allocation
//
/----------------------------------------------------------------------*/

InferenceWeight_p InferenceWeightsAlloc(void)
{
   InferenceWeight_p handle = InferenceWeightCellAlloc();

   (*handle)[PCLOpNoOp]            = PCL_OP_NOOP_WEIGHT;
   (*handle)[PCLOpInitial]         = PCL_OP_INITIAL_WEIGHT;
   (*handle)[PCLOpQuote]           = PCL_OP_QUOTE_WEIGHT;
   (*handle)[PCLOpParamod]         = PCL_OP_PARAMOD_WEIGHT;
   (*handle)[PCLOpSimParamod]      = PCL_OP_SIM_PARAMOD_WEIGHT;
   (*handle)[PCLOpEvalGC]          = PCL_OP_EVALGC_WEIGHT;
   (*handle)[PCLOpEResolution]     = PCL_OP_ERESOLUTION_WEIGHT;
   (*handle)[PCLOpEFactoring]      = PCL_OP_EFACTORING_WEIGHT;
   (*handle)[PCLOpSimplifyReflect] = PCL_OP_SIMPLIFYREFLECT_WEIGHT;
   (*handle)[PCLOpContextSimplifyReflect] = PCL_OP_CONTEXTSIMPLIFYREFLECT_WEIGHT;
   (*handle)[PCLOpACResolution]    = PCL_OP_ACRESOLUTION_WEIGHT;
   (*handle)[PCLOpRewrite]         = PCL_OP_REWRITE_WEIGHT;
   (*handle)[PCLOpURewrite]        = PCL_OP_UREWRITE_WEIGHT;
   (*handle)[PCLOpClauseNormalize] = PCL_OP_CLAUSENORMALIZE_WEIGHT;
   (*handle)[PCLOpSplitClause]     = PCL_OP_SPLITCLAUSE_WEIGHT;

  return handle;
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprUpdateRefs()
//
//   Given a PCL expression, update the counter in all leaves according
//   to the inferences they directly paricipate in.
//
// Global Variables: -
//
// Side Effects    : Updates those counters.
//
/----------------------------------------------------------------------*/

void PCLExprUpdateRefs(PCLProt_p prot, PCLExpr_p expr)
{
   PCLStep_p handle;
   int i;

   /* printf("PCLExprUpdateRefs(%p %p), %ld\n", prot, expr,
      expr->arg_no);    PCLExprPrint(stdout, expr, false); printf("\n"); */

   switch(expr->op)
   {
   case PCLOpNoOp:
   case PCLOpInitial:
   case PCLOpQuote:
    /* Do nothing */
    break;
   case PCLOpParamod:
   case PCLOpSimParamod:
    assert(expr->arg_no == 2);
    if((handle = PCLExprGetQuotedArg(prot,expr,0)))
    {
       handle->other_generating_refs++;
    }
         else
         {
            PCLExprUpdateRefs(prot, PCLExprArg(expr,0));
         }
    if((handle = PCLExprGetQuotedArg(prot,expr,1)))
    {
       handle->active_pm_refs++;
    }
         else
         {
            PCLExprUpdateRefs(prot, PCLExprArg(expr,1));
         }
    break;
   case PCLOpEResolution:
   case PCLOpEFactoring:
   case PCLOpSplitClause: /* No idea if this one belongs here. but I
              have no better idea */
    if((handle = PCLExprGetQuotedArg(prot,expr,0)))
    {
       handle->other_generating_refs++;
    }
         else
         {
            PCLExprUpdateRefs(prot, PCLExprArg(expr,0));
         }
    break;
   case PCLOpSimplifyReflect:
   case PCLOpACResolution:
   case PCLOpRewrite:
   case PCLOpURewrite:
   case PCLOpClauseNormalize:
    if((handle = PCLExprGetQuotedArg(prot,expr,0)))
    {
       handle->passive_simpl_refs++;
    }
         else
         {
            PCLExprUpdateRefs(prot, PCLExprArg(expr,0));
         }
    for(i=1; i < expr->arg_no; i++)
    {
       if((handle = PCLExprGetQuotedArg(prot,expr,i)))
       {
          handle->active_simpl_refs++;
       }
            else
            {
               PCLExprUpdateRefs(prot, PCLExprArg(expr,i));
            }
    }
    break;
   default:
         /* Nothing happens for the FOF inference types */
    /* assert(false); */
         break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: PCLStepUpdateRefs()
//
//   Update reference counters from this step to its parents.
//
// Global Variables: -
//
// Side Effects    : Just those
//
/----------------------------------------------------------------------*/

void PCLStepUpdateRefs(PCLProt_p prot, PCLStep_p step)
{
   /* PCLStepPrint(stdout, step);printf("\n"); */
   if(step->just->op == PCLOpQuote)
   {
      PCLStep_p handle = PCLProtFindStep(prot, PCLExprArg(step->just,0));
      handle->pure_quote_refs++;
   }
   PCLExprUpdateRefs(prot, step->just);
}



/*-----------------------------------------------------------------------
//
// Function: PCLProtUpdateRefs()
//
//   For all steps in prot update the reference counters
//
// Global Variables: -
//
// Side Effects    : As described
//
/----------------------------------------------------------------------*/

void PCLProtUpdateRefs(PCLProt_p prot)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepUpdateRefs(prot,step);
   }
}



/*-----------------------------------------------------------------------
//
// Function: PCLStepLemmaCmpWrapper()
//
//   Wrapper for PCLStepLemmaCmp in IntOrP's
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int PCLStepLemmaCmpWrapper(const void* s1, const void* s2)
{
   const IntOrP* step1 = (const IntOrP*) s1;
   const IntOrP* step2 = (const IntOrP*) s2;

   return PCLStepLemmaCmp(step1->p_val, step2->p_val);
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepLemmaCmp()
//
//   Compare the lemma rating of two PCL steps, returning -1, 0, 1
//   depending on outcome.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

int PCLStepLemmaCmp(PCLStep_p step1, PCLStep_p step2)
{
   float r1, r2;

   r1 = step1->lemma_quality;
   r2 = step2->lemma_quality;
   if(r1 < r2)
   {
      return -1;
   }
   if(r2 < r1)
   {
      return 1;
   }
   return 0;
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprProofSize()
//
//   Compute the proof size of the expression (including proofs for
//   children). Assumes that all previous steps already have correct
//   weight.
//
// Global Variables: -
//
// Side Effects    : May reorganize step tree.
//
/----------------------------------------------------------------------*/

long PCLExprProofSize(PCLProt_p prot, PCLExpr_p expr, InferenceWeight_p iw,
            bool use_lemmas)
{
   long res = 0,i;
   PCLStep_p step;

   assert(prot);
   assert(expr);

   if(expr->op == PCLOpQuote)
   {
      step = PCLProtFindStep(prot, PCLExprArg(expr,0));
      if(step)
      {
    res = PCLStepProofSize(prot, step, iw, use_lemmas);
      }
      else
      {
    PCLExprPrint(stderr, expr, false); fprintf(stderr, "  ");
    Error("Reference to non-existing step", SYNTAX_ERROR);
      }
   }
   else if(expr->op == PCLOpInitial)
   {
      res = (*iw)[expr->op];
   }
   else
   {
      res = (*iw)[expr->op];
      for(i=0; i<expr->arg_no; i++)
      {
    res+=PCLExprProofSize(prot, PCLExprArg(expr,i), iw, use_lemmas);
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepProofSize()
//
//   Compute and return the proof size of step. Caches result in the
//   step. If use_lemmas is true, always return 0
//
// Global Variables: -
//
// Side Effects    : As noted
//
/----------------------------------------------------------------------*/

long PCLStepProofSize(PCLProt_p prot, PCLStep_p step, InferenceWeight_p iw,
            bool use_lemmas)
{
   assert(prot);
   assert(step);

   if(step->proof_tree_size == PCLNoWeight)
   {
      assert(step->just);
      step->proof_tree_size = PCLExprProofSize(prot, step->just, iw, use_lemmas);
   }
   if(use_lemmas && PCLStepQueryProp(step, PCLIsLemma))
   {
      return 0;
   }
   else
   {
      return step->proof_tree_size;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtComputeProofSize()
//
//   Compute proof weight for all steps. If use_lemmas is true, assume
//   proof weight of lemmas is 0 (but still record it).
//
// Global Variables: -
//
// Side Effects    : As noted
//
/----------------------------------------------------------------------*/

void PCLProtComputeProofSize(PCLProt_p prot, InferenceWeight_p iw,
              bool use_lemmas)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      assert(step);
      PCLStepProofSize(prot, step, iw, use_lemmas);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepComputeLemmaWeight()
//
//   Compute the lemma quality of a PCL step based on the information
//   stored in it.
//
// Global Variables: -
//
// Side Effects    : Caches the result in step->lemma_quality
//
/----------------------------------------------------------------------*/

float PCLStepComputeLemmaWeight(PCLProt_p prot, PCLStep_p step,
            LemmaParam_p params)
{
   float res;

   if(PCLStepQueryProp(step, PCLIsFOFStep))
   {
      res = 0;
   }
   else
   {
      res =
         (1.0+
          params->tree_base_weight +
          step->active_pm_refs        * params->act_pm_w +
          step->other_generating_refs * params->o_gen_w +
          step->active_simpl_refs     * params->act_simpl_w +
          step->passive_simpl_refs    * params->pas_simpl_w)
         *
         (1.0+
          step->proof_tree_size
            )
         /
         (params->size_base_weight+ClauseStandardWeight(step->logic.clause));

      if(ClauseIsHorn(step->logic.clause))
      {
         res = res*params->horn_bonus;
      }

      if((step->passive_simpl_refs || step->pure_quote_refs)
         &&
         !(step->active_pm_refs+step->other_generating_refs+step->active_simpl_refs) )
      {
         res = 0;
      }
      else
      {
         if(ClauseIsTrivial(step->logic.clause))
         {
            res = 0;
         }
      }
   }
   step->lemma_quality = res;
   return res;
}

/*-----------------------------------------------------------------------
//
// Function: PCLProtComputeLemmaWeights()
//
//   Compute the lemma rating for all steps. Return the step with the
//   best lemma rating.
//
// Global Variables: -
//
// Side Effects    : Caches lemma rating in the steps.
//
/----------------------------------------------------------------------*/

PCLStep_p PCLProtComputeLemmaWeights(PCLProt_p prot, LemmaParam_p params)
{
   PCLStep_p step, res=NULL;
   float best_rating = -1, current_rating;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      current_rating = PCLStepComputeLemmaWeight(prot, step, params);
      if(current_rating > best_rating && !PCLStepQueryProp(step, PCLIsLemma))
      {
    best_rating = current_rating;
    res = step;
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtSeqFindLemmas()
//
//   Mark all lemmas in procol which have a lemma rating of at least
//   quality_limit, but not more than max_number. Goes from first to
//   last step, taking already marked lemmas into account. Assumes
//   topologically ordered protocol (otherwise lemma ratings might be
//   off). Returns number of lemmas found.
//
// Global Variables: -
//
// Side Effects    : Marks lemmata, changes weights in steps.
//
/----------------------------------------------------------------------*/


long PCLProtSeqFindLemmas(PCLProt_p prot, LemmaParam_p params,
           InferenceWeight_p iw, long max_number,
           float quality_limit)
{
   PStackPointer i;
   PCLStep_p step;
   long res = 0;

   PCLProtSerialize(prot);
   PCLProtResetTreeData(prot, false);
   PCLProtUpdateRefs(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepProofSize(prot, step, iw, true);
      if(PCLStepComputeLemmaWeight(prot, step, params) >= quality_limit)
      {
    PCLStepSetProp(step, PCLIsLemma);
    res++;
    if(res > max_number)
    {
       break;
    }
      }
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtRecFindLemmas()
//
//   Recursively mark lemmas in prot as follows: Find the globally
//   best one, mark it. Recalculate all weight. Repeat. Return number
//   of lemmas found. Terminate if max_number lemmas have
//   been found or quality drops below quality limit.
//
// Global Variables: -
//
// Side Effects    : Changes lemma weights.
//
/----------------------------------------------------------------------*/

long PCLProtRecFindLemmas(PCLProt_p prot, LemmaParam_p params,
             InferenceWeight_p iw, long max_number,
             float quality_limit)
{
   long i;
   PCLStep_p lemma = NULL;

   PCLProtResetTreeData(prot, false);
   PCLProtUpdateRefs(prot);
   for(i=0; i<max_number; i++)
   {
      PCLProtResetTreeData(prot, true);
      PCLProtComputeProofSize(prot, iw, true);
      lemma = PCLProtComputeLemmaWeights(prot, params);
      if(!lemma || (lemma->lemma_quality < quality_limit))
      {
    break;
      }
      PCLStepSetProp(lemma, PCLIsLemma);
   }
   return i;
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtFlatFindLemmas()
//
//   Find lemmas by computing scores once, sorting (by score) and
//   picking the best lemmas (down to quality_limit). Returns number
//   of lemmas selected.
//
// Global Variables: -
//
// Side Effects    : Changes weight, reorders protocol.
//
/----------------------------------------------------------------------*/

long PCLProtFlatFindLemmas(PCLProt_p prot, LemmaParam_p params,
            InferenceWeight_p iw, long max_number,
            float quality_limit)
{
   long i;
   PCLStep_p step = NULL;

   PCLProtResetTreeData(prot, false);
   PCLProtUpdateRefs(prot);
   PCLProtComputeProofSize(prot, iw, true);
   PCLProtComputeLemmaWeights(prot, params);
   PCLProtSerialize(prot);
   prot->is_ordered = false;
   PStackSort(prot->in_order, PCLStepLemmaCmpWrapper);

   max_number = MIN(max_number, PStackGetSP(prot->in_order));
   for(i=0; i<max_number; i++)
   {
      step = PStackPopP(prot->in_order);
      if(step->lemma_quality < quality_limit)
      {
    break;
      }
      PCLStepSetProp(step, PCLIsLemma);
   }
   return i;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


