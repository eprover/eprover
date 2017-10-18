/*-----------------------------------------------------------------------

File  : pcl_analysis.c

Author: Stephan Schulz

Contents

  Functions for performing some analysis on PCL files (primarily for
  reenabling the old learning code with the new output format).

  Copyright 2004 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Feb  3 23:28:50 CET 2004
    New

-----------------------------------------------------------------------*/

#include "pcl_analysis.h"



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
// Function: pcl_example_cmp()
//
//   Compare two PCL steps as follows: All proof steps are equal and
//   smaller than all non-proof steps. Non-proof steps are compared by
//   gen_ref/(sim_ref+1).
//
// Global Variables:
//
// Side Effects    :
//
/----------------------------------------------------------------------*/

int pcl_example_cmp(const void* ex1, const void* ex2)
{
   const IntOrP* e1 = (const IntOrP*) ex1;
   const IntOrP* e2 = (const IntOrP*) ex2;
   PCLStep_p s1, s2;
   double w1, w2;

   s1 = e1->p_val;
   s2 = e2->p_val;

   if(PCLStepQueryProp(s1, PCLIsProofStep))
   {
      if(PCLStepQueryProp(s2, PCLIsProofStep))
      {
         return 0;
      }
      return -1;
   }
   else if(PCLStepQueryProp(s2, PCLIsProofStep))
   {
      return 1;
   }
   assert(!PCLStepQueryProp(s2, PCLIsProofStep));
   assert(!PCLStepQueryProp(s1, PCLIsProofStep));
   w1 = s1->useless_gen_refs/(float)(s1->useless_simpl_refs+1);
   w2 = s2->useless_gen_refs/(float)(s2->useless_simpl_refs+1);
   if(w1<w2)
   {
      return 1;
   }
   if(w1>w2)
   {
      return -1;
   }
   return 0;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: PCLExprProofDistance()
//
//   Find the longest inference chain from the nearest proof clause
//   referenced in the expression. If no proof clause is among its
//   ancestors, return LONG_MAX. Assumes that proof clauses are
//   marked!
//
// Global Variables: -
//
// Side Effects    : Sets proof_distance in parents (indirectly)
//
/----------------------------------------------------------------------*/

long PCLExprProofDistance(PCLProt_p prot, PCLExpr_p expr)
{
   long res = 0, tmp;
   PCLStep_p step;
   int i;

   switch(expr->op)
   {
   case PCLOpNoOp:
    assert(false);
    break;
   case PCLOpInitial:
         res = PCL_PROOF_DIST_DEFAULT;
    break;
   case PCLOpQuote:
    step = PCLProtFindStep(prot,PCLExprArg(expr,0));
         res = PCLStepProofDistance(prot, step);
    break;
   default:
    for(i=0; i<expr->arg_no; i++)
    {
            tmp = PCLExprProofDistance(prot, PCLExprArg(expr,i));
       res = MAX(res,tmp);
    }
         if(res!=PCL_PROOF_DIST_INFINITY)
         {
            res++;
         }
    break;
   }
   return res;
}


/*-----------------------------------------------------------------------
//
// Function: PCLStepProofDistance()
//
//   Find the longest inference chain from the nearest proof clause
//   referenced in the steps expression (or 0 if step is proof
//   step). If no proof clause is among its
//   ancestors, return LONG_MAX. Assumes that proof clauses are
//   marked! Non-proof initial clauses get PCL_PROOF_DIST_DEFAULT.
//
// Global Variables: -
//
// Side Effects    : Sets proof_distance in parents and step
//
/----------------------------------------------------------------------*/


long PCLStepProofDistance(PCLProt_p prot, PCLStep_p step)
{
   if(step->proof_distance == PCL_PROOF_DIST_UNKNOWN)
   {
      if(PCLStepQueryProp(step,PCLIsProofStep))
      {
         step->proof_distance = 0;
      }
      else
      {
         step->proof_distance = PCLExprProofDistance(prot,step->just);
      }
   }
   return step->proof_distance;
}



/*-----------------------------------------------------------------------
//
// Function: PCLProtProofDistance()
//
//   Compute the proof distance for all steps in protocol. Assumes
//   that proof steps are already identified.
//
// Global Variables: -
//
// Side Effects    : Sets proof_distance in all steps.
//
/----------------------------------------------------------------------*/

void PCLProtProofDistance(PCLProt_p prot)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepProofDistance(prot, step);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLExprUpdateGRefs()
//
//   Update the reference counters in all parents of expr
//   appropriately.
//
// Global Variables: -
//
// Side Effects    : See above.
//
/----------------------------------------------------------------------*/

void PCLExprUpdateGRefs(PCLProt_p prot, PCLExpr_p expr, bool proofstep)
{
   int i;
   PCLStep_p parent;

   switch(expr->op)
   {
   case PCLOpInitial:
   case PCLOpQuote:
   case PCLOpClauseNormalize:
         break;
   case PCLOpParamod:
   case PCLOpSimParamod:
   case PCLOpEResolution:
   case PCLOpEFactoring:
   case PCLOpSplitClause:
         for(i=0; i<expr->arg_no; i++)
    {
            parent = PCLExprGetQuotedArg(prot,expr, i);
            if(parent)
            {
               if(proofstep)
               {
                  parent->contrib_gen_refs++;
               }
               else
               {
                  parent->useless_gen_refs++;
               }
            }
            else
            {
               PCLExprUpdateGRefs(prot, PCLExprArg(expr,i), proofstep);
            }
         }
         break;
   case PCLOpSimplifyReflect:
   case PCLOpContextSimplifyReflect:
   case PCLOpRewrite:
   case PCLOpURewrite:
   case PCLOpACResolution:
         PCLExprUpdateGRefs(prot, PCLExprArg(expr,0), proofstep);
         for(i=1; i<expr->arg_no; i++)
    {
            parent = PCLExprGetQuotedArg(prot,expr, i);
            if(parent)
            {
               if(proofstep)
               {
                  parent->contrib_simpl_refs++;
               }
               else
               {
                  parent->useless_simpl_refs++;
               }
            }
            else
            {
               PCLExprUpdateGRefs(prot, PCLExprArg(expr,i), proofstep);
            }
         }
         break;
   default:
         /* These are unknown inferences (typcally FOF), we ignore
            them. */
         break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtUpdateGRefs()
//
//   For all steps, mark how often they are used to generate or
//   simplify proof or non-proof clauses.  Assumes
//   that proof steps are already identified.
//
// Global Variables: -
//
// Side Effects    : Updates reference counters
//
/----------------------------------------------------------------------*/

void PCLProtUpdateGRefs(PCLProt_p prot)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot);

   for(i=0; i<PStackGetSP(prot->in_order); i++)
   {
      step = PStackElementP(prot->in_order, i);
      PCLStepUpdateGRefs(prot, step);
   }
}


/*-----------------------------------------------------------------------
//
// Function: PCLProtSelectExamples()
//
//   Select examples for pattern-based learning. Selects all proof
//   clauses and up to neg_examples negative examples. Negative
//   examples are selected by ratio of generating to simplifying
//   applications (generating  bad, simplification good). Returns
//   number of steps selected.
//
// Global Variables: -
//
// Side Effects    : Temporarily rearranges the protocol, sets
//                   PCLIsExample in selected steps.
//
/----------------------------------------------------------------------*/

long PCLProtSelectExamples(PCLProt_p prot, long neg_examples)
{
   PCLStep_p step;
   PStackPointer i;

   PCLProtSerialize(prot); /* Ensure its all on the stack */
   prot->is_ordered = false;
   qsort(prot->in_order->stack,
         PStackGetSP(prot->in_order),
         sizeof(IntOrP),
         pcl_example_cmp);
   for(i=0; (i<PStackGetSP(prot->in_order))&&(neg_examples>0); i++)
   {
      step = PStackElementP(prot->in_order, i);
      if(PCLStepQueryProp(step, PCLIsFOFStep))
      {
         continue; /* We only consider clauses */
      }
      PCLStepSetProp(step, PCLIsExample);
      if(!PCLStepQueryProp(step, PCLIsProofStep))
      {
         neg_examples--;
      }
   }
   return i;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


