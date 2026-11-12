/*-----------------------------------------------------------------------

  File  : ccl_featuresets.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Sparse clause features for subsumption indexing: a feature set
  maps feature numbers to (positive) values. Features not in the set
  are implicitly 0. Unlike feature vectors (ccl_freqvectors), there
  is no limit on the number of symbols.

  Which features are computed is selected by a FeatureSetType:

  FSAllFeatures: the features of the dense "All" feature vectors,
    i.e. the number of positive and negative literals, and for each
    function symbol its number of occurrences and maximal depth in
    positive and negative literals.
  FSAllMinDepthFeatures: the above plus, for each symbol and sign,
    its minimal depth (see below).
  FSPlainFeatures: only the maximal and minimal depths. Unlike
    literal and symbol counts, these are compatible with plain (set)
    subsumption, where several literals of the subsumer may be mapped
    to the same literal.

  Minimal depth: instantiation keeps the depth of existing
  occurrences and can only add new ones, and a subsumed clause
  contains (an instance of) all literals of the subsumer, so the
  minimal depth of a symbol in the subsumed clause is at most that in
  the subsumer. To get a feature that can only grow, the value is
  FS_MINDEPTH_BASE-depth (at least 1), and 0 if the symbol does not
  occur. There is one exception: a positive unit equation s=t also
  subsumes u[s']=u[t'] for instances s', t' (LiteralSubsumesClause()),
  which moves its symbols deeper. Therefore positive minimal depths
  are only taken from positive non-equational literals (a predicate
  literal only ever subsumes a predicate literal, at the top), and a
  positive unit equation has no minimal depth features at all.

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

#include <stdint.h>
#include <ccl_clauses.h>
#include <ccl_ftsplit.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Feature numbers: the literal counts come first, then six features
   for each function symbol (f_code 0 collects phony applications in
   HO terms, as for feature vectors). */

typedef enum
{
   FSAllFeatures,
   FSAllMinDepthFeatures,
   FSPlainFeatures
}FeatureSetType;

typedef enum
{
   FSNegFreq     = 0,
   FSPosFreq     = 1,
   FSPosDepth    = 2,
   FSNegDepth    = 3,
   FSPosMinDepth = 4,
   FSNegMinDepth = 5
}FSSymbolFeature;

#define FS_POS_LITS        0
#define FS_NEG_LITS        1
#define FS_LIT_FEATURES    2
#define FS_SYMBOL_FEATURES 6
#define FS_MINDEPTH_BASE   (1<<20)
#define FSSymbolFeatureNo(f_code, kind) \
   (FS_LIT_FEATURES+FS_SYMBOL_FEATURES*(f_code)+(kind))

/* A feature with its (non-zero) value. Feature numbers and values
   fit into 32 bits (values are counts, depths, and FS_MINDEPTH_BASE
   minus a depth). */

typedef struct fs_pair_cell
{
   int32_t feature;
   int32_t value;
}FSPairCell, *FSPair_p;

/* A set of features with non-zero values, as an array of pairs
   sorted by feature number. Sets are built once from a clause and
   never changed afterwards. */

typedef struct feature_set_cell
{
   long      size;      /* Number of features in the set */
   FSPair_p  pairs;     /* NULL if size is 0 */
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
   (FEATURESETCELL_MEM+(set)->size*(long)sizeof(FSPairCell))

void         FeatureSetFree(FeatureSet_p junk);
long         FeatureSetGetVal(FeatureSet_p set, long feature);

FeatureSet_p ClauseFeatureSetCompute(Clause_p clause, FeatureSetType type);
FeatureSetType FeatureSetTypeParse(const char* name);

bool         FeatureSetLessEqual(FeatureSet_p set1, FeatureSet_p set2);
long         FeatureSetFirstDiff(FeatureSet_p set1, FeatureSet_p set2);
void         FeatureSetSplitConsider(FeatureSet_p set1, FeatureSet_p set2,
                                     FTSplit_p split, long *best,
                                     long *best_score);
#define      FeatureSetEqual(set1, set2) \
   (FeatureSetFirstDiff((set1), (set2)) == -1)

void         FeatureSetPrint(FILE* out, FeatureSet_p set);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
