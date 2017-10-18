/*-----------------------------------------------------------------------

File  : cle_numfeatures.h

Author: Stephan Schulz

Contents

  Functions and data types for dealing with numerical features of the
  clause set. This is, unfortunatly, not quite orthogonal to
  che_clausesetfeatures.h at the moment.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Jul 26 18:47:37 MET DST 1999
    New

-----------------------------------------------------------------------*/

#ifndef CHE_NUMFEATURES

#define CHE_NUMFEATURES

#include <ccl_clausesets.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


#define FEATURE_NUMBER 15

/* Preliminary list of features to use:

   - Number of unit clauses
   - Number of nonunit horn  clauses
   - Number of nonhorn general clauses
   - Average term depth of positive literals, standard deviation
   - Average term depth of negative literals, standard deviation
   - Average term size of positive literals, standard deviation
   - Average term size of negative literals, standard deviation
   - Average number of positive literals, standard deviation
   - Average number of negative literals, standard deviation

   */

#define FEATURE_NUMBER 15

typedef struct featurescell
{
   long      pred_max_arity;
   PDArray_p pred_distrib;
   long      func_max_arity;
   PDArray_p func_distrib;
   double    features[FEATURE_NUMBER];
}FeaturesCell, *Features_p;

#define SEL_FEATURE_WEIGHTS {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, \
                             1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}
#define SEL_PRED_WEIGHT 1.0
#define SEL_FUNC_WEIGHT 1.0


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FeaturesCellAlloc() (FeaturesCell*)SizeMalloc(sizeof(FeaturesCell))
#define FeaturesCellFree(junk) SizeFree(junk, sizeof(FeaturesCell))

Features_p FeaturesAlloc(void);
void       FeaturesFree(Features_p junk);

void ComputeClauseSetNumFeatures(Features_p features, ClauseSet_p set,
             Sig_p sig);

void       NumFeaturesPrint(FILE* out, Features_p features);
Features_p NumFeaturesParse(Scanner_p in);

double     NumFeatureDistance(Features_p f1, Features_p f2, double
               pred_w, double func_w, double* weights);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





