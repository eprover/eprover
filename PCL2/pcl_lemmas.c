/*-----------------------------------------------------------------------

File  : pcl_lemmas.c

Author: Stephan Schulz

Contents

  Data structures and functions for the detection of lemmas in PCL2
  protocol. 

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
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

/*-----------------------------------------------------------------------
//
// Function: pcl_expr_get_quoted_arg()
//
//   If the designated arg is a quote expression, retrieve and return
//   the quoted step. Otherwise return NULL.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static PCLStep_p pcl_expr_get_quoted_arg(PCLProt_p prot, PCLExpr_p expr, 
					 int arg)
{
   PCLExpr_p argexpr;

   assert(arg < expr->arg_no);

   /* printf("pcl_expr_get_quoted_arg(%p, %p, %d)...\n", prot, expr,
      arg); */
   argexpr = PCLExprArg(expr,arg);
   if(argexpr->op == PCLOpQuote)
   {
      return PCLProtFindStep(prot,PCLExprArg(argexpr,0));
   }
   return NULL;
}

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

   handle->base_weight  = LEMMA_BASE_W;
   handle->act_pm_w     = LEMMA_ACT_PM_W;
   handle->o_gen_w      = LEMMA_O_GEN_W;
   handle->act_simpl_w  = LEMMA_ACT_SIMPL_W;
   handle->pas_simpl_w  = LEMMA_PAS_SIMPL_W;
   handle->proof_tree_w = LEMMA_PROOF_TREE_W;
   handle->proof_dag_w  = LEMMA_PROOF_DAG_W;

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

InferenceWeight_p InferenceWeightsAlloc()
{
   InferenceWeight_p handle = InferenceWeightCellAlloc();
      
   (*handle)[PCLOpNoOp]            = PCL_OP_NOOP_WEIGHT;
   (*handle)[PCLOpInitial]         = PCL_OP_INITIAL_WEIGHT;
   (*handle)[PCLOpQuote]           = PCL_OP_QUOTE_WEIGHT;
   (*handle)[PCLOpParamod]         = PCL_OP_PARAMOD_WEIGHT;
   (*handle)[PCLOpEResolution]     = PCL_OP_ERESOLUTION_WEIGHT;
   (*handle)[PCLOpEFactoring]      = PCL_OP_EFACTORING_WEIGHT;
   (*handle)[PCLOpSimplifyReflect] = PCL_OP_SIMPLIFYREFLECT_WEIGHT;
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
//   Given a PCL expression, update the counter in all leave according
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
	 assert(expr->arg_no == 2);
	 if((handle = pcl_expr_get_quoted_arg(prot,expr,0)))
	 {
	    handle->other_generating_refs++;
	 }
	 if((handle = pcl_expr_get_quoted_arg(prot,expr,1)))
	 {
	    handle->active_pm_refs++;	    
	 }
	 break;
   case PCLOpEResolution:
   case PCLOpEFactoring:
   case PCLOpSplitClause: /* No idea if this one belongs here. but I
			     have no better idead */
	 if((handle = pcl_expr_get_quoted_arg(prot,expr,0)))
	 {
	    handle->other_generating_refs++;
	 }
	 break;
   case PCLOpSimplifyReflect:
   case PCLOpACResolution:
   case PCLOpRewrite:
   case PCLOpURewrite:
   case PCLOpClauseNormalize:
	 if((handle = pcl_expr_get_quoted_arg(prot,expr,0)))
	 {
	    handle->passive_simpl_refs++;
	 }
	 for(i=1; i < expr->arg_no; i++)
	 {
	    if((handle = pcl_expr_get_quoted_arg(prot,expr,i)))
	    {
	       handle->active_simpl_refs++;
	    }	    
	 }
	 break;	 
   default:
	 assert(false);
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

   if(expr->op == PCLOpQuote)
   {
      step = PCLProtFindStep(prot,PCLExprArg(expr,0));
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
   if(step->proof_tree_size == PCLNoWeight)
   {
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
   
   /* printf("PCLProtComputeProofSize()...\n"); */

   PCLProtSerialize(prot);
   
   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
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

   res =    
      (params->base_weight +
       step->active_pm_refs        * params->act_pm_w + 
       step->other_generating_refs * params->o_gen_w + 
       step->active_simpl_refs     * params->act_simpl_w + 
       step->passive_simpl_refs    * params->pas_simpl_w)
      *
      (step->proof_tree_size)
      /
      ClauseStandardWeight(step->clause);

   if(step->passive_simpl_refs && 
      (!step->active_pm_refs+step->other_generating_refs+step->active_simpl_refs)) 
   {
      res = 0;
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
//PCLProtComputeLemmaWeights
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
      if(current_rating > best_rating)
      {
	 best_rating = current_rating;
	 res = step;
      }
   }
   return res;
}



/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


