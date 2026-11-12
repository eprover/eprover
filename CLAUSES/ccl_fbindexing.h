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


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FBIndexCellAlloc()    (FBIndexCell*)SizeMalloc(sizeof(FBIndexCell))
#define FBIndexCellFree(junk) SizeFree(junk, sizeof(FBIndexCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define FBINDEX_MEM 48
#else
#define FBINDEX_MEM MEMSIZE(FBIndexCell)
#endif

void FBIndexFree(FBIndex_p junk);
void FBIndexInsert(FBIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause);
bool FBIndexDelete(FBIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause);
long FBIndexMaxWeight(FBIndex_p index);
long FBIndexMinWeight(FBIndex_p index);
long FBIndexCountNodes(FBIndex_p index, bool leaves);
void FBIndexPrint(FILE* out, FBIndex_p index, bool fullterms);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
