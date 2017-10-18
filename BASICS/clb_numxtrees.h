/*-----------------------------------------------------------------------

File  : clb_numxtrees.h

Author: Stephan Schulz

Contents

  Definitions for SPLAY trees with long integer keys and vectors of
  IntOrPs as values. Copied from clb_numtrees.h

  Copyright 1998, 1999, 2011 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Mon Aug  1 11:04:53 CEST 2011
    New from clb_numtrees.h

-----------------------------------------------------------------------*/

#ifndef CLB_NUMXTREES

#define CLB_NUMXTREES

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


#define NUMXTREEVALUES 4

/* General purpose data structure for indexing objects by a numerical
   key. Integer values are supported directly, for all other objects
   pointers can be used (and need to be casted carefully by the
   wrapper functions). Objects pointed to by the value fields are not
   part of the data stucture and will not be touched by deallocating
   trees or tree nodes. */

typedef struct numxtreecell
{
   long                key;
   IntOrP              vals[NUMXTREEVALUES];
   struct numxtreecell *lson;
   struct numxtreecell *rson;
}NumXTreeCell, *NumXTree_p;


#define NumXTreeCellAlloc() (NumXTreeCell*)SizeMalloc(sizeof(NumXTreeCell))
#define NumXTreeCellFree(junk)        SizeFree(junk, sizeof(NumXTreeCell))


NumXTree_p NumXTreeCellAllocEmpty(void);
void      NumXTreeFree(NumXTree_p junk);
NumXTree_p NumXTreeInsert(NumXTree_p *root, NumXTree_p newnode);
bool       NumXTreeStore(NumXTree_p *root, long key, IntOrP val1, IntOrP val2);
NumXTree_p NumXTreeFind(NumXTree_p *root, long key);
NumXTree_p NumXTreeExtractEntry(NumXTree_p *root, long key);
NumXTree_p NumXTreeExtractRoot(NumXTree_p *root);
bool      NumXTreeDeleteEntry(NumXTree_p *root, long key);
long      NumXTreeNodes(NumXTree_p root);
NumXTree_p NumXTreeMaxNode(NumXTree_p root);
#define   NumXTreeMaxKey(tree) (NumXTreeMaxNode(tree)->key)

PStack_p NumXTreeLimitedTraverseInit(NumXTree_p root, long limit);

AVL_TRAVERSE_DECLARATION(NumXTree, NumXTree_p)
#define NumXTreeTraverseExit(stack) PStackFree(stack)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





