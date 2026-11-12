/*-----------------------------------------------------------------------

  File  : ccl_fsindexing.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Feature set tree indexing for subsumption: the same tree as in
  ccl_ftindexing, but over sparse feature sets (ccl_featuresets)
  instead of dense feature vectors. Clauses are stored at leaves
  with identical feature sets, and decision nodes branch on the
  value of an arbitrary feature. Only features that actually
  distinguish stored sets are tested.

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

/* A node of the feature set tree. A decision node tests one feature
   (by its number) and has one successor for each value of that
   feature that occurs among the clauses below it (at least two). A
   leaf (final node) stores all clauses with one particular feature
   set. It owns one copy of this set, and the clauses stored at the
   leaf reference it (as their feature_set) while they are in the
   index. The tests on the path to a leaf need not cover
   all features, so retrieval has to check the leaf set. Every set
   has a unique path, so there is at most one leaf for each set. */

typedef struct fs_index_cell
{
   bool         final;
   long         clause_count;  /* Clauses at or below this node */
   long         feature;       /* Only for decision nodes, else -1 */
   FeatureSet_p fset;          /* Only for final nodes, else NULL */
   union
   {
      IntMap_p successors;
      OrdSet_p clauses;
   }u1;
}FSIndexCell, *FSIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FSIndexCellAlloc()    (FSIndexCell*)SizeMalloc(sizeof(FSIndexCell))
#define FSIndexCellFree(junk) SizeFree(junk, sizeof(FSIndexCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define FSINDEX_MEM 24
#else
#define FSINDEX_MEM MEMSIZE(FSIndexCell)
#endif

void FSIndexFree(FSIndex_p junk);
void FSIndexInsert(FSIndex_p *root, long *storage, Clause_p clause);
bool FSIndexDelete(FSIndex_p *root, long *storage, Clause_p clause);
long FSIndexCountNodes(FSIndex_p index, bool leaves);
void FSIndexPrint(FILE* out, FSIndex_p index, bool fullterms);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
