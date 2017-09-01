/*-----------------------------------------------------------------------

  File  : che_specsigfeatures.h

  Author: Stephan Schulz

  Contents

  Definitions for determining various features of specifications,
  i.e. clause and (later) formula sets. This is analoguous to
  che_clausesetfeatures.[ch], but uses different features.

  Copyright 2017 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Created: Wed Aug 30 11:40:34 CEST 2017

-----------------------------------------------------------------------*/

#ifndef CHE_SPECSIGFEATURES

#define CHE_SPECSIGFEATURES

#include <ccl_proofstate.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/



//   Structure of features (L = SIG_FEATURE_ARITY_LIMIT)
//   For axioms (=non-conjectures)
//   features[0]: Number of positive equational literals
//   features[1]: Number of negative equational literals
//   For positive literals:
//   features[...2+L]: Frequency of of pred-symbols of arity n
//   features[...2+2L]: Frequency of fun-symbols of arity n
//   features[...2+3L]: Max depth of fun-symbols of arity n
//   For negative literals:
//   features[...2+4L]: Frequency of of pred-symbols of arity n
//   features[...2+5L]: Frequency of fun-symbols of arity n
//   features[...2+6L]: Max depth of fun-symbols of arity n
//   ...followed by the same for conjectures/negated conjectures
//   ...folowed by counters for unit, horn, and general clauses


// F-Count, P-Count, F-Depth
#define SPECSIG_SIGFTRS      (3*SIG_FEATURE_ARITY_LIMIT)

// Two eq-literal counts+the previous for positive/negative literals
#define SPECSIG_CS_FTRS      (2+2*SPECSIG_SIGFTRS)

// The above for axioms/conjectures, + clause counts, + F/P signature counts
#define SPECSIG_TOTAL_FTR_NO ((2*SPECSIG_CS_FTRS)+3+(2*SIG_FEATURE_ARITY_LIMIT))

#define SPECSIG_POS_EL_OFFSET 0
#define SPECSIG_NEG_EL_OFFSET 1
#define SPECSIG_SYMD_OFFSET   2

#define SPECSIG_AX_FTRS      (0*SPECSIG_CS_FTRS)
#define SPECSIG_AX_POSEQ     (SPECSIG_AX_FTRS+SPECSIG_POS_EL_OFFSET)
#define SPECSIG_AX_NEGEQ     (SPECSIG_AX_FTRS+SPECSIG_NEG_EL_OFFSET)
#define SPECSIG_AX_SYMD      (SPECSIG_AX_FTRS+SPECSIG_SYMD_OFFSET)
#define SPECSIG_AX_SYMD_POS  (SPECSIG_AX_SYMD)
#define SPECSIG_AX_SYMD_NEG  (SPECSIG_AX_SYMD_POS+SPECSIG_SIGFTRS)

#define SPECSIG_CJ_FTRS      (1*SPECSIG_CS_FTRS)
#define SPECSIG_CJ_POSEQ     (SPECSIG_CJ_FTRS+SPECSIG_POS_EL_OFFSET)
#define SPECSIG_CJ_NEGEQ     (SPECSIG_CJ_FTRS+SPECSIG_NEG_EL_OFFSET)
#define SPECSIG_CJ_SYMD      (SPECSIG_CJ_FTRS+SPECSIG_SYMD_OFFSET)
#define SPECSIG_CJ_SYMD_POS  (SPECSIG_CJ_SYMD)
#define SPECSIG_CJ_SYMD_NEG  (SPECSIG_CJ_SYMD_POS+SPECSIG_SIGFTRS)

#define SPECSIG_GLOBAL_FTRS  (2*SPECSIG_CS_FTRS)
#define SPECSIG_GLOBAL_UNIT  SPECSIG_GLOBAL_FTRS
#define SPECSIG_GLOBAL_HORN  (SPECSIG_GLOBAL_FTRS+1)
#define SPECSIG_GLOBAL_GNRL  (SPECSIG_GLOBAL_FTRS+2)

#define SPECSIG_GLOBAL_SIG   ((2*SPECSIG_CS_FTRS)+3)

typedef struct spec_sig_feature_cell
{
   long features[SPECSIG_TOTAL_FTR_NO];
}SpecSigFeatureCell, *SpecSigFeature_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define SpecSigFeatureCellAlloc() \
   (SpecSigFeatureCell*)SizeMalloc(sizeof(SpecSigFeatureCell))
#define SpecSigFeatureCellFree(junk) SizeFree(junk, sizeof(SpecSigFeatureCell))

void SpecSigFeatureInit(SpecSigFeature_p specftrs);
void SpecSigFeaturePrint(FILE*out, SpecSigFeature_p specftrs);

/* Note: Only use 3*SIG_FEATURE_ARITY_LIMIT values from features! */
void TermCollectSigFeatures(Sig_p sig, Term_p term, long* features);

#define EqnCollectSigFeatures(eqn, features)                            \
    TermCollectSigFeatures((eqn)->bank->sig, (eqn->lterm), (features));\
    TermCollectSigFeatures((eqn)->bank->sig, (eqn->rterm), (features))


/* Note: Only use 2+6*SIG_FEATURE_ARITY_LIMIT values from features */
void ClauseCollectSigFeatures(Clause_p clause, long* features);
void ClauseComputeSigFeatures(Clause_p clause, long* features);

/* Note: Use the full SpecSigFeatureCell */

void ClauseSetCollectSigFeatures(Sig_p sig, ClauseSet_p set,
                                 SpecSigFeature_p specftrs);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
