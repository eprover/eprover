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

#ifndef NUMARRTREEVALUES
#define NUMARRTREEVALUES 8
#endif

/* General purpose data structure for indexing objects by a numerical
   key. Integer values are supported directly, for all other objects
   pointers can be used (and need to be casted carefully by the
   wrapper functions). Objects pointed to by the value fields are not
   part of the data stucture and will not be touched by deallocating
   trees or tree nodes. */

typedef struct numarrtreecell
{
   long                key;
   IntOrP              vals[NUMARRTREEVALUES];
   struct numarrtreecell *lson;
   struct numarrtreecell *rson;
}NumArrTreeCell, *NumArrTree_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define NumArrTreeCellAlloc() (NumArrTreeCell*)SizeMalloc(sizeof(NumArrTreeCell))
#define NumArrTreeCellFree(junk)        SizeFree(junk, sizeof(NumArrTreeCell))

#define NUMARRTREECELL_MEM MEMSIZE(NumArrTreeCell)


bool      NumArrTreeNodeSingleton(NumArrTree_p node, int index);
NumArrTree_p NumArrTreeCellAllocEmpty(void);
void      NumArrTreeFree(NumArrTree_p junk);
NumArrTree_p NumArrTreeInsertNode(NumArrTree_p *root, NumArrTree_p newnode);
NumArrTree_p NumArrTreeInsertKeyValPair(NumArrTree_p *root, long key, void* val);
bool       NumArrTreeStoreNode(NumArrTree_p *root, long key, IntOrP val);
NumArrTree_p NumArrTreeFindNode(NumArrTree_p *root, long key);
NumArrTree_p NumArrTreePrepDel(NumArrTree_p *root, long key);
NumArrTree_p NumArrTreeMaxNode(NumArrTree_p root);
long      NumArrTreeMaxKey(NumArrTree_p node);

PStack_p NumArrTreeLimitedTraverseInit(NumArrTree_p root, long limit);

NumArrTree_p NumArrTreeTraverseNext(PStack_p state, long* last_seen_key);
#define NumArrTreeTraverseExit(stack) PStackFree(stack)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





