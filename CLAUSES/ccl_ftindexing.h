/*-----------------------------------------------------------------------

  File  : ccl_ftindexing.h

  Author: Stephan Schulz (schulz@eprover.org)

  Contents

  Feature tree indexing for subsumption: an alternative to the
  feature vector trie (ccl_fcvindexing). Clauses are stored at leaves
  with identical feature vectors, and decision nodes branch on the
  value of an arbitrary feature. Only features that actually
  distinguish stored vectors are tested.

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Nov 12 2026

  -----------------------------------------------------------------------*/

#ifndef CCL_FTINDEXING

#define CCL_FTINDEXING

#include <clb_intmap.h>
#include <ccl_freqvectors.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* A node of the feature tree. A decision node tests one feature (its
   position in the vector) and has one successor for each value of
   that feature that occurs among the clauses below it (at least
   two). A leaf (final node) stores all clauses with one particular
   feature vector. It owns one copy of this vector, and the clauses
   stored at the leaf reference it (as their feature_vec) while they
   are in the index. The tests on the path to a leaf need not cover
   all features, so retrieval has to check the leaf vector. Every
   vector has a unique path, so there is at most one leaf for each
   vector. */

typedef struct ft_index_cell
{
   bool         final;
   long         clause_count;  /* Clauses at or below this node */
   long         feature;       /* Only for decision nodes, else -1 */
   FreqVector_p vec;           /* Only for final nodes, else NULL */
   union
   {
      IntMap_p successors;
      OrdSet_p clauses;
   }u1;
}FTIndexCell, *FTIndex_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define FTIndexCellAlloc()    (FTIndexCell*)SizeMalloc(sizeof(FTIndexCell))
#define FTIndexCellFree(junk) SizeFree(junk, sizeof(FTIndexCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define FTINDEX_MEM 24
#else
#define FTINDEX_MEM MEMSIZE(FTIndexCell)
#endif

void FTIndexFree(FTIndex_p junk);
void FTIndexInsert(FTIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause);
bool FTIndexDelete(FTIndex_p *root, long *storage, FTSplit_p split,
                   Clause_p clause);
long FTIndexCountNodes(FTIndex_p index, bool leaves);
void FTIndexPrint(FILE* out, FTIndex_p index, bool fullterms);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
