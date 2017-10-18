/*-----------------------------------------------------------------------

File  : clb_quadtrees.h

Author: Stephan Schulz

Contents

  Trees indexed by 4 words (two pointers and two integers). See
  clb_ptrees.h (and below) for details.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

Changes

<1> Tue Jan  4 00:53:38 MET 2000
    Copied and modified clb_ptrees.h

-----------------------------------------------------------------------*/

#ifndef CLB_QUADTREES

#define CLB_QUADTREES

#include <clb_ptrees.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* The Quadkey type: 4 words (two pointers, two ints) */

typedef struct quadkeycell
{
   void*               p1;
   int                 i1;
   void*               p2;
   int                 i2;
}QuadKeyCell,QuadKey,*QuadKey_p;

/* Quadtree cell: We have the 4-part key and a single value. */

typedef struct quadtreecell
{
   QuadKey             key;
   IntOrP              val;
   struct quadtreecell *lson;
   struct quadtreecell *rson;
}QuadTreeCell, *QuadTree_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define QuadTreeCellAlloc()    (QuadTreeCell*)SizeMalloc(sizeof(QuadTreeCell))
#define QuadTreeCellFree(junk) SizeFree(junk, sizeof(QuadTreeCell))

int        DoubleKeyCmp(void* p1, int i1, void *p2, int i2);
int        QuadKeyCmp(QuadKey_p p1, QuadKey_p p2);
void       QuadTreeFree(QuadTree_p junk);
QuadTree_p QuadTreeInsert(QuadTree_p *root, QuadTree_p newnode);
bool       QuadTreeStore(QuadTree_p *root, QuadKey_p key, IntOrP val);
QuadTree_p QuadTreeFind(QuadTree_p *root, QuadKey_p key);
QuadTree_p QuadTreeExtractEntry(QuadTree_p *root, QuadKey_p key);
bool       QuadTreeDeleteEntry(QuadTree_p *root, QuadKey_p key);

AVL_TRAVERSE_DECLARATION(QuadTree, QuadTree_p)
#define QuadTreeTraverseExit(stack) PStackFree(stack)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
