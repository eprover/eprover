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

#include <ccl_freqvectors.h>
#include <clb_intmap.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


PERF_CTR_DECL(FVIndexTimer);


typedef struct fvindex_parms_cell
{
   FVCollectCell cspec;
   bool use_perm_vectors;
   bool eliminate_uninformative;
   long max_symbols;
   long symbol_slack;
}FVIndexParmsCell, *FVIndexParms_p;



typedef struct fv_index_cell
{
   bool     final;
   long     clause_count;
   union
   {
      IntMap_p successors;
      PTree_p  clauses;
   }u1;
}FVIndexCell, *FVIndex_p;

typedef struct fvi_anchor_cell
{
   FVCollect_p  cspec;
   PermVector_p perm_vector;
   FVIndex_p    index;
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

FVIAnchor_p FVIAnchorAlloc(FVCollect_p cspec, PermVector_p perm);
void        FVIAnchorFree(FVIAnchor_p junk);

#ifdef CONSTANT_MEM_ESTIMATE
#define FVINDEX_MEM 16
#else
#define FVINDEX_MEM MEMSIZE(FVIndexCell)
#endif

#define FVIndexStorage(index) ((index)?(index)->storage:0)

FVIndex_p   FVIndexGetNextNonEmptyNode(FVIndex_p node, long key);
void        FVIndexInsert(FVIAnchor_p index, FreqVector_p vec_clause);

bool        FVIndexDelete(FVIAnchor_p index, Clause_p clause);

long        FVIndexCountNodes(FVIndex_p index, bool leaves, bool empty);

FVPackedClause_p FVIndexPackClause(Clause_p clause, FVIAnchor_p anchor);

void        FVIndexPrint(FILE* out, FVIndex_p index, bool fullterms);
#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
