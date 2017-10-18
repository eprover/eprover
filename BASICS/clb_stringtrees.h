/*-----------------------------------------------------------------------

File  : clb_stringtrees.h

Author: Stephan Schulz

Contents

  Definitions for AVL trees with string keys and up to two int or
  pointer values. Part of the implementation is based on public domain
  code by D.D. Sleator.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Thu Sep 25 02:23:01 MET DST 1997
    New

-----------------------------------------------------------------------*/

#ifndef CLB_STRINGTREES

#define CLB_STRINGTREES

#include <clb_dstrings.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/



/* General purpose data structure for indexing objects by a string
   key. Integer values are supported directly, for all other objects
   pointers can be used (and need to be casted carefully by the
   wrapper functions). Keys are considered to be part of the tree and
   will be FREE'd by memory deallocation. Objects pointed to by the
   value fields are not part of the data stucture and will not be
   touched by deallocating trees or tree nodes. */

typedef struct strtreecell
{
   char               *key;
   IntOrP             val1;
   IntOrP             val2;
   struct strtreecell *lson;
   struct strtreecell *rson;
}StrTreeCell, *StrTree_p;


#define StrTreeCellAlloc() (StrTreeCell*)SizeMalloc(sizeof(StrTreeCell))
#define StrTreeCellFree(junk)        SizeFree(junk, sizeof(StrTreeCell))

StrTree_p StrTreeCellAllocEmpty(void);
void      StrTreeFree(StrTree_p junk);
StrTree_p StrTreeInsert(StrTree_p *root, StrTree_p newnode);
StrTree_p StrTreeStore(StrTree_p *root, char* key, IntOrP val1, IntOrP
             val2);
StrTree_p StrTreeFind(StrTree_p *root, const char* key);
StrTree_p StrTreeExtractEntry(StrTree_p *root, const char* key);
bool      StrTreeDeleteEntry(StrTree_p *root, const char* key);

AVL_TRAVERSE_DECLARATION(StrTree, StrTree_p)
#define StrTreeTraverseExit(stack) PStackFree(stack)


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





