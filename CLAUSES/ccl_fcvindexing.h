/*-----------------------------------------------------------------------

File  : ccl_fcvindexing.h

Author: Stephan Schulz

Contents

  Functions for handling frequency count vector indexing for clause
  subsumption.

  Copyright 2003 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jul  1 13:05:36 CEST 2003
    New
<2> Sun Feb  6 02:16:41 CET 2005 (actually 2 weeks or so earlier)
    Switched to IntMap

-----------------------------------------------------------------------*/

#ifndef CCL_FCVINDEXING

#define CCL_FCVINDEXING

#if USE_INTMAPOLD
   #include <clb_intmap_old.h>
#else
   #include <clb_intmap.h>
#endif

#include <ccl_freqvectors.h>
#include <ccl_ftindexing.h>
#include <ccl_fsindexing.h>
#include <ccl_fbindexing.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


PERF_CTR_DECL(FVIndexTimer);


/* The data structure used for subsumption indexing: a trie over
   feature vectors (FVIndex), a feature tree over feature vectors
   (FTIndex), a feature tree over feature sets (FSIndex), or a binary
   feature tree over feature sets (FBIndex). */

typedef enum
{
   FVITrie,
   FVITree,
   FVISetTree,
   FVIBinSetTree
}FVIStructure;

#define FVIUsesFeatureSets(structure) \
   (((structure) == FVISetTree) || ((structure) == FVIBinSetTree))

typedef struct fvindex_parms_cell
{
   FVCollectCell cspec;
   bool use_perm_vectors;
   bool eliminate_uninformative;
   long max_symbols;
   long symbol_slack;
   FVIStructure structure;
   FeatureSetType set_features; /* Only for FVISetTree */
   FTSplitRule split_rule;      /* Only for FVITree and FVISetTree */
   long bucket_size;            /* Only for (Bin)SetTree */
   bool weight_bounds;          /* Only for (Bin)SetTree */
   bool feature_lists;          /* Only for BinSetTree */
}FVIndexParmsCell, *FVIndexParms_p;



/* A node of the index. The path from the root to a leaf (final
   node) spells out a feature vector. The leaf owns one copy of this
   vector, and all clauses stored at the leaf reference it (as their
   feature_vec) while they are in the index. */

typedef struct fv_index_cell
{
   bool         final;
   long         clause_count;
   FreqVector_p vec;           /* Only for final nodes, else NULL */
   union
   {
      IntMap_p successors;
      OrdSet_p  clauses;
   }u1;
}FVIndexCell, *FVIndex_p;

/* The anchor describes how feature vectors are computed and holds
   the index proper, as selected by structure: a feature vector trie
   (index), a feature tree over feature vectors (ftindex, see
   ccl_ftindexing.h), a feature tree over feature sets (fsindex,
   see ccl_fsindexing.h), or a binary one (fbindex, see
   ccl_fbindexing.h). Feature sets do not depend on cspec and
   perm_vector, set_features selects their features. split selects
   the tests of new decision nodes in the feature trees. */

typedef struct fvi_anchor_cell
{
   FVCollect_p  cspec;
   PermVector_p perm_vector;
   FVIStructure structure;
   FeatureSetType set_features;
   FVIndex_p    index;        /* Only for FVITrie */
   FTIndex_p    ftindex;      /* Only for FVITree, NULL if empty */
   FSIndex_p    fsindex;      /* Only for FVISetTree, NULL if empty */
   FBIndex_p    fbindex;      /* Only for FVIBinSetTree, NULL if empty */
   FBLists_p    fblists;      /* Only for FVIBinSetTree, optional */
   FTSplit_p    split;        /* Not for FVITrie */
   bool         weight_bounds; /* Prune (Bin)SetTree retrieval for
                                  multiset subsumption by weight */
   long         storage;
}FVIAnchorCell, *FVIAnchor_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

/* extern FVIndexParmsCell FVIDefaultParameters; */

#define FVIndexParmsCellAlloc() (FVIndexParmsCell*)SizeMalloc(sizeof(FVIndexParmsCell))
#define FVIndexParmsCellFree(junk) SizeFree(junk, sizeof(FVIndexParmsCell))

void           FVIndexParmsInit(FVIndexParms_p parms);
FVIndexParms_p FVIndexParmsAlloc(void);
#define FVIndexParmsFree(junk) FVIndexParmsCellFree(junk)

#define FVIndexCellAlloc()    (FVIndexCell*)SizeMalloc(sizeof(FVIndexCell))
#define FVIndexCellFree(junk) SizeFree(junk, sizeof(FVIndexCell))

FVIndex_p FVIndexAlloc(void);
void      FVIndexFree(FVIndex_p junk);

#define FVIAnchorCellAlloc()    (FVIAnchorCell*)SizeMalloc(sizeof(FVIAnchorCell))
#define FVIAnchorCellFree(junk) SizeFree(junk, sizeof(FVIAnchorCell))

FVIAnchor_p FVIAnchorAlloc(FVCollect_p cspec, PermVector_p perm,
                           FVIndexParms_p parms);
void        FVIAnchorFree(FVIAnchor_p junk);

#ifdef CONSTANT_MEM_ESTIMATE
#define FVINDEX_MEM 16
#else
#define FVINDEX_MEM MEMSIZE(FVIndexCell)
#endif

#define FVIndexStorage(index) ((index)?(index)->storage:0)

FVIndex_p   FVIndexGetNextNonEmptyNode(FVIndex_p node, long key);
void        FVIndexInsert(FVIAnchor_p index, Clause_p clause);

bool        FVIndexDelete(FVIAnchor_p index, Clause_p clause);

long        FVIndexCountNodes(FVIndex_p index, bool leaves, bool empty);
void        FVIAnchorPrintStats(FILE* out, char* name, FVIAnchor_p anchor);

FreqVector_p FVIndexComputeVector(FVIAnchor_p anchor, Clause_p clause);
void         ClauseComputeFeatures(Clause_p clause, FVIAnchor_p anchor);
void         ClauseDropFeatures(Clause_p clause);
FreqVector_p ClauseGetFeatureVector(Clause_p clause, FVIAnchor_p anchor,
                                    bool *is_temp);
FeatureSet_p ClauseGetFeatureSet(Clause_p clause, FVIAnchor_p anchor,
                                 bool *is_temp);
#ifndef NDEBUG
bool         ClauseFeaturesAreValid(Clause_p clause, FVIAnchor_p anchor);
#endif

void        FVIndexPrint(FILE* out, FVIndex_p index, bool fullterms);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
