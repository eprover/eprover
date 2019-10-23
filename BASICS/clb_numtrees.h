/*-----------------------------------------------------------------------

  File  : clb_numtrees.h

  Author: Stephan Schulz

  Contents

  Definitions for SPLAY trees with long integer keys and up to two
  long or pointer values. Copied from clb_stringtrees.h

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Sep 25 02:23:01 MET DST 1997

  -----------------------------------------------------------------------*/

#ifndef CLB_NUMTREES

#define CLB_NUMTREES

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* General purpose data structure for indexing objects by a numerical
   key. Integer values are supported directly, for all other objects
   pointers can be used (and need to be casted carefully by the
   wrapper functions). Objects pointed to by the value fields are not
   part of the data stucture and will not be touched by deallocating
   trees or tree nodes. */

typedef struct numtreecell
{
   long               key;
   IntOrP             val1;
   IntOrP             val2;
   struct numtreecell *lson;
   struct numtreecell *rson;
}NumTreeCell, *NumTree_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define NumTreeCellAlloc() (NumTreeCell*)SizeMalloc(sizeof(NumTreeCell))
#define NumTreeCellFree(junk)        SizeFree(junk, sizeof(NumTreeCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define NUMTREECELL_MEM 24
#else
#define NUMTREECELL_MEM MEMSIZE(NumTreeCell)
#endif

NumTree_p NumTreeCellAllocEmpty(void);
void      NumTreeFree(NumTree_p junk);
NumTree_p NumTreeInsert(NumTree_p *root, NumTree_p newnode);
bool      NumTreeStore(NumTree_p *root, long key, IntOrP val1, IntOrP val2);
long      NumTreeDebugPrint(FILE* out, NumTree_p tree,
                            bool keys_only);
NumTree_p NumTreeFind(NumTree_p *root, long key);
NumTree_p NumTreeExtractEntry(NumTree_p *root, long key);
NumTree_p NumTreeExtractRoot(NumTree_p *root);
bool      NumTreeDeleteEntry(NumTree_p *root, long key);
long      NumTreeNodes(NumTree_p root);
NumTree_p NumTreeMaxNode(NumTree_p root);
#define   NumTreeMaxKey(tree) (NumTreeMaxNode(tree)->key)

PStack_p NumTreeLimitedTraverseInit(NumTree_p root, long limit);

AVL_TRAVERSE_DECLARATION(NumTree, NumTree_p)
#define NumTreeTraverseExit(stack) PStackFree(stack)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
