/*-----------------------------------------------------------------------

  File  : ccl_fbindexing.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Binary feature set tree indexing for subsumption: a variant of the
  feature set tree (ccl_fsindexing) in which every decision node
  compares one feature against a threshold and has exactly two
  successors (values up to the threshold, and values above it).
  Leaves hold buckets of up to bucket size different feature sets
  (see ccl_ftsplit.h), which are split as evenly as possible when
  they overflow.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#ifndef CCL_FBINDEXING

#define CCL_FBINDEXING

#include <ccl_fsindexing.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* A node of the binary feature set tree. A decision node sends
   clauses whose value for feature is at most threshold to le, all
   others to gt (both always exist). A leaf (final node) stores up to
   the bucket size entries (see ccl_fsindexing.h) with different
   feature sets, in insertion order. The tests on the path to a leaf
   need not cover all features, so retrieval has to check the entry
   sets. Every set has a unique path, so there is at most one entry
   for each set. Weight bounds are as for ccl_fsindexing. */

typedef struct fb_index_cell
{
   bool         final;
   long         clause_count;  /* Clauses at or below this node */
   long         feature;       /* Only for decision nodes, else -1 */
   long         threshold;     /* Only for decision nodes */
   long         entry_count;   /* Only for final nodes, else 0 */
   long         max_weight;    /* Or FS_WEIGHT_INVALID */
   long         min_weight;    /* Or FS_WEIGHT_INVALID */
   union
   {
      struct
      {
         struct fb_index_cell *le;
         struct fb_index_cell *gt;
      }succ;
      FSEntry_p entries;
   }u1;
}FBIndexCell, *FBIndex_p;

/* Optional per-feature lists of the leaf entries of a binary feature
   set tree: for each feature, the entries whose set contains it (in
   insertion order), and their number. A clause subsumed by a query
   clause has all features of the query's set, so its entry is on the
   list of each of them, in particular on the shortest one. */

typedef struct fb_lists_cell
{
   PDArray_p entries;  /* Feature -> OrdSet_p of FSEntry_p */
   PDArray_p sizes;    /* Feature -> number of entries */
   long      cells;    /* Total number of list cells */
}FBListsCell, *FBLists_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FBIndexCellAlloc()    (FBIndexCell*)SizeMalloc(sizeof(FBIndexCell))
#define FBIndexCellFree(junk) SizeFree(junk, sizeof(FBIndexCell))
#define FBListsCellAlloc()    (FBListsCell*)SizeMalloc(sizeof(FBListsCell))
#define FBListsCellFree(junk) SizeFree(junk, sizeof(FBListsCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define FBINDEX_MEM 48
#else
#define FBINDEX_MEM MEMSIZE(FBIndexCell)
#endif

FBLists_p FBListsAlloc(void);
void      FBListsFree(FBLists_p junk);
OrdSet_p  FBListsShortest(FBLists_p lists, FeatureSet_p fset);

void FBIndexFree(FBIndex_p junk);
void FBIndexInsert(FBIndex_p *root, long *storage, FTSplit_p split,
                   FBLists_p lists, Clause_p clause);
bool FBIndexDelete(FBIndex_p *root, long *storage, FTSplit_p split,
                   FBLists_p lists, Clause_p clause);
long FBIndexMaxWeight(FBIndex_p index);
long FBIndexMinWeight(FBIndex_p index);
long FBIndexCountNodes(FBIndex_p index, bool leaves);
void FBIndexCollectStats(FBIndex_p index, FTStats_p stats);
long FBListsStorage(FBLists_p lists);
void FBIndexPrint(FILE* out, FBIndex_p index, bool fullterms);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
