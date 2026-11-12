/*-----------------------------------------------------------------------

  File  : ccl_fsindexing.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Feature set tree indexing for subsumption: the same tree as in
  ccl_ftindexing, but over sparse feature sets (ccl_featuresets)
  instead of dense feature vectors. Clauses are stored at leaves
  with identical feature sets, and decision nodes branch on the
  value of an arbitrary feature. Only features that actually
  distinguish stored sets are tested. Leaves can hold several
  different sets (buckets), and nodes bound the weights of the
  clauses below them.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#ifndef CCL_FSINDEXING

#define CCL_FSINDEXING

#include <clb_intmap.h>
#include <ccl_featuresets.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* A leaf entry: all clauses in the leaf with one particular feature
   set. The entry owns this set, and its clauses reference it (as
   their feature_set) while they are in the index. */

typedef struct fs_entry_cell
{
   FeatureSet_p         fset;
   long                 clause_count;
   OrdSet_p             clauses;
   struct fs_entry_cell *next;
}FSEntryCell, *FSEntry_p;

/* A node of the feature set tree. A decision node tests one feature
   (by its number) and has one successor for each value of that
   feature that occurs among the clauses below it (at least two). A
   leaf (final node) stores up to the bucket size (see ccl_ftsplit.h)
   entries with different feature sets, in insertion order; when
   there are more, it is replaced by a decision node. The tests on
   the path to a leaf need not cover all features, so retrieval has
   to check the entry sets. Every set has a unique path, so there is
   at most one entry for each set.

   Each node also bounds the standard weights of the clauses at or
   below it. The bounds are kept up to date on insertion, invalidated
   (FS_WEIGHT_INVALID) on deletion of a clause with an extremal
   weight, and recomputed on demand (FSIndexMaxWeight(),
   FSIndexMinWeight()). */

#define FS_WEIGHT_INVALID -1

typedef struct fs_index_cell
{
   bool         final;
   long         clause_count;  /* Clauses at or below this node */
   long         feature;       /* Only for decision nodes, else -1 */
   long         entry_count;   /* Only for final nodes, else 0 */
   long         max_weight;    /* Or FS_WEIGHT_INVALID */
   long         min_weight;    /* Or FS_WEIGHT_INVALID */
   union
   {
      IntMap_p  successors;
      FSEntry_p entries;
   }u1;
}FSIndexCell, *FSIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FSIndexCellAlloc()    (FSIndexCell*)SizeMalloc(sizeof(FSIndexCell))
#define FSIndexCellFree(junk) SizeFree(junk, sizeof(FSIndexCell))
#define FSEntryCellAlloc()    (FSEntryCell*)SizeMalloc(sizeof(FSEntryCell))
#define FSEntryCellFree(junk) SizeFree(junk, sizeof(FSEntryCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define FSINDEX_MEM 40
#define FSENTRY_MEM 32
#else
#define FSINDEX_MEM MEMSIZE(FSIndexCell)
#define FSENTRY_MEM MEMSIZE(FSEntryCell)
#endif

FSEntry_p FSEntryAlloc(long *storage, FeatureSet_p fset);
void      FSEntryFree(long *storage, FSEntry_p junk);

void FSIndexFree(FSIndex_p junk);
void FSIndexInsert(FSIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause);
bool FSIndexDelete(FSIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause);
long FSIndexCountNodes(FSIndex_p index, bool leaves);
long FSIndexMaxWeight(FSIndex_p index);
long FSIndexMinWeight(FSIndex_p index);
void FSIndexPrint(FILE* out, FSIndex_p index, bool fullterms);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
