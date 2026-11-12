/*-----------------------------------------------------------------------

  File  : ccl_featuresets.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Sparse clause features for subsumption indexing: a feature set
  maps feature numbers to (positive) values. Features not in the set
  are implicitly 0. Unlike feature vectors (ccl_freqvectors), there
  is no limit on the number of symbols.

  The features are those of the dense "All" feature vectors: the
  number of positive and negative literals, and for each function
  symbol its number of occurrences and maximal depth in positive and
  negative literals.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#ifndef CCL_FEATURESETS

#define CCL_FEATURESETS

#include <clb_numtrees.h>
#include <ccl_clauses.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Feature numbers: the literal counts come first, then four features
   for each function symbol (f_code 0 collects phony applications in
   HO terms, as for feature vectors). */

typedef enum
{
   FSNegFreq  = 0,
   FSPosFreq  = 1,
   FSPosDepth = 2,
   FSNegDepth = 3
}FSSymbolFeature;

#define FS_POS_LITS        0
#define FS_NEG_LITS        1
#define FS_LIT_FEATURES    2
#define FS_SYMBOL_FEATURES 4
#define FSSymbolFeatureNo(f_code, kind) \
   (FS_LIT_FEATURES+FS_SYMBOL_FEATURES*(f_code)+(kind))

/* A set of features with non-zero values, as a NumTree with the
   feature number as key and the value in val1.i_val. */

typedef struct feature_set_cell
{
   long      size;      /* Number of features in the set */
   NumTree_p features;
}FeatureSetCell, *FeatureSet_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FeatureSetCellAlloc()    (FeatureSetCell*)SizeMalloc(sizeof(FeatureSetCell))
#define FeatureSetCellFree(junk) SizeFree(junk, sizeof(FeatureSetCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define FEATURESETCELL_MEM 16
#else
#define FEATURESETCELL_MEM MEMSIZE(FeatureSetCell)
#endif

#define FeatureSetStorage(set) \
   (FEATURESETCELL_MEM+(set)->size*NUMTREECELL_MEM)

FeatureSet_p FeatureSetAlloc(void);
void         FeatureSetFree(FeatureSet_p junk);
long         FeatureSetGetVal(FeatureSet_p set, long feature);
void         FeatureSetAddVal(FeatureSet_p set, long feature, long value);
void         FeatureSetMaxVal(FeatureSet_p set, long feature, long value);

FeatureSet_p ClauseFeatureSetCompute(Clause_p clause);

bool         FeatureSetLessEqual(FeatureSet_p set1, FeatureSet_p set2);
long         FeatureSetFirstDiff(FeatureSet_p set1, FeatureSet_p set2);
#define      FeatureSetEqual(set1, set2) \
   (FeatureSetFirstDiff((set1), (set2)) == -1)

void         FeatureSetPrint(FILE* out, FeatureSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
