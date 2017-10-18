/*-----------------------------------------------------------------------

File  : pcl_lemmas.h

Author: Stephan Schulz

Contents

  Definition for dealing with lemmas in PCL protocols.


Copyright 1998-2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Sun Jun 15 22:47:43 CEST 2003
    New

-----------------------------------------------------------------------*/

#ifndef PCL_LEMMAS

#define PCL_LEMMAS

#include <pcl_protocol.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Lemma rating is as follows:

   size   = StandardWeight(lemma)
   actpm  = references as active partner in paramod
   o_gen  = references from other generating inferences
   act_simpl = references from active simplification
   pas_simpl = references from being simplified
   subsum = references from subsumption (will probably not always be
              available)
   proof_tree = size of proof tree (unfolded)
   proof_dag  = size of proof tree seen as a dag


   (1+
   actpm*actpm_w +
   o_gen*o_gen_w +
   act_simpl*act_simpl_w +
   pas_simpl*pas_simpl_w +
   subsum*subsum_w)
   *
   (1+
   proof_tree*proof_tree_w+
   proof_dag*proof_dag_w)
   /
   1+size*size_w

   Large is good! */

typedef struct lemma_param_cell
{
   long   tree_base_weight;
   double act_pm_w;
   double o_gen_w;
   double act_simpl_w;
   double pas_simpl_w;
   double proof_tree_w;
   double proof_dag_w;
   long   size_base_weight;
   double horn_bonus;
}LemmaParamCell, *LemmaParam_p;

#define LEMMA_TREE_BASE_W  1
#define LEMMA_ACT_PM_W     2.0
#define LEMMA_O_GEN_W      1.0
#define LEMMA_ACT_SIMPL_W  2.0
#define LEMMA_PAS_SIMPL_W  1.0
#define LEMMA_PROOF_TREE_W 1.0
#define LEMMA_PROOF_DAG_W  0.0 /* Don't know how to efficiently compute
               * DAG size at the moment */
#define LEMMA_SIZE_BASE_W  1
#define LEMMA_HORN_BONUS_W 2.0

typedef long InferenceWeightType[PCLOpMaxOp];
typedef InferenceWeightType *InferenceWeight_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define LemmaParamCellAlloc() (LemmaParamCell*)SizeMalloc(sizeof(LemmaParamCell))
#define LemmaParamCellFree(junk)        SizeFree(junk, sizeof(LemmaParamCell))
LemmaParam_p LemmaParamAlloc(void);
#define LemmaParamFree(cell) LemmaParamCellFree(cell)

#define InferenceWeightCellAlloc()\
        (InferenceWeight_p)SizeMalloc(sizeof(InferenceWeightType))
#define InferenceWeightCellFree(junk) \
        SizeFree(junk, sizeof(InferenceWeightType))
InferenceWeight_p InferenceWeightsAlloc(void);
#define InferenceWeightsFree(junk) InferenceWeightCellFree(junk)

void PCLExprUpdateRefs(PCLProt_p prot, PCLExpr_p expr);
void PCLStepUpdateRefs(PCLProt_p prot, PCLStep_p step);
void PCLProtUpdateRefs(PCLProt_p prot);

int PCLStepLemmaCmpWrapper(const void* s1, const void* s2);
int PCLStepLemmaCmp(PCLStep_p step1, PCLStep_p step2);

long PCLExprProofSize(PCLProt_p prot, PCLExpr_p expr, InferenceWeight_p iw,
            bool use_lemmas);
long PCLStepProofSize(PCLProt_p prot, PCLStep_p step, InferenceWeight_p iw,
            bool use_lemmas);
void PCLProtComputeProofSize(PCLProt_p prot, InferenceWeight_p iw,
              bool use_lemmas);

float PCLStepComputeLemmaWeight(PCLProt_p prot, PCLStep_p step,
            LemmaParam_p params);
PCLStep_p PCLProtComputeLemmaWeights(PCLProt_p prot, LemmaParam_p params);

long PCLProtSeqFindLemmas(PCLProt_p prot, LemmaParam_p params,
           InferenceWeight_p iw, long max_number,
           float quality_limit);
long PCLProtRecFindLemmas(PCLProt_p prot, LemmaParam_p params,
           InferenceWeight_p iw, long max_number,
           float quality_limit);
long PCLProtFlatFindLemmas(PCLProt_p prot, LemmaParam_p params,
            InferenceWeight_p iw, long max_number,
            float quality_limit);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





