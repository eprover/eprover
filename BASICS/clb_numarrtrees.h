/*-----------------------------------------------------------------------

File  : clb_numarrtrees.h

Author: Albert Eisfeld

Contents

  Definitions for SPLAY trees with long integer keys and arrays of
  IntOrPs as values. Copied and modified from clb_numxtrees.h

  Copyright 2026 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Aug  1 11:04:53 CEST 2011
    New from clb_numtxrees.h
TODO change date on all relevant files
-----------------------------------------------------------------------*/

#ifndef CLB_NUMARRTREES

#define CLB_NUMARRTREES

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>
#include <clb_pstacks.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* Size of newly created nodes. Two adjacent nodes of the same size
   whose union is aligned to twice that size (buddies) are merged, up
   to NUMARRTREEMAXVALUES values per node. Node sizes are always
   powers of two and a node of size s starts at a multiple of s, so
   keys are split into node key and slot with bit masks. */

#ifndef NUMARRTREEVALUES
#define NUMARRTREEVALUES 8
#endif

#ifndef NUMARRTREEMAXVALUES
#define NUMARRTREEMAXVALUES 4096
#endif

#if (NUMARRTREEVALUES < 1) || (NUMARRTREEVALUES & (NUMARRTREEVALUES - 1))
#error "NUMARRTREEVALUES must be a power of two"
#endif
#if (NUMARRTREEMAXVALUES < NUMARRTREEVALUES) || (NUMARRTREEMAXVALUES & (NUMARRTREEMAXVALUES - 1))
#error "NUMARRTREEMAXVALUES must be a power of two >= NUMARRTREEVALUES"
#endif

#define NumArrTreeNodeKey(key) ((key) & ~(long)(NUMARRTREEVALUES - 1))

/* General purpose data structure for indexing objects by a numerical
   key. Integer values are supported directly, for all other objects
   pointers can be used (and need to be casted carefully by the
   wrapper functions). Objects pointed to by the value fields are not
   part of the data stucture and will not be touched by deallocating
   trees or tree nodes. Each node holds the values for the keys
   key..key+size-1. */

typedef struct numarrtreecell
{
   long                key;
   long                size;
   struct numarrtreecell *lson;
   struct numarrtreecell *rson;
   IntOrP              vals[];
}NumArrTreeCell, *NumArrTree_p;

#define NumArrTreeSlot(node, key) ((key) & ((node)->size - 1))

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define NumArrTreeCellMem(size) (sizeof(NumArrTreeCell)+(size)*sizeof(IntOrP))
#define NumArrTreeCellAlloc(size) \
   (NumArrTreeCell*)SizeMalloc(NumArrTreeCellMem(size))
#define NumArrTreeCellFree(junk) SizeFree(junk, NumArrTreeCellMem((junk)->size))

/* Estimated memory per value, for the IntMap storage estimate */
#define NUMARRTREECELL_MEM \
   (MEMSIZE(NumArrTreeCell)+NUMARRTREEVALUES*sizeof(IntOrP))


bool      NumArrTreeNodeEmpty(NumArrTree_p node);
NumArrTree_p NumArrTreeCellAllocEmpty(long key, long size);
void      NumArrTreeFree(NumArrTree_p junk);
NumArrTree_p NumArrTreeInsertKeyValPair(NumArrTree_p *root, long key, void* val);
NumArrTree_p NumArrTreeFindNode(NumArrTree_p *root, long key);
void*     NumArrTreeDeleteKey(NumArrTree_p *root, long key);
NumArrTree_p NumArrTreeMaxNode(NumArrTree_p root);
long      NumArrTreeMaxKey(NumArrTree_p node);

PStack_p NumArrTreeLimitedTraverseInit(NumArrTree_p root, long limit);

NumArrTree_p NumArrTreeTraverseNext(PStack_p state);
#define NumArrTreeTraverseExit(stack) PStackFree(stack)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





