/*-----------------------------------------------------------------------

File  : clb_ptrees.h

Author: Stephan Schulz

Contents
 
  Data structures for the efficient management of pointer
  sets. I substituted this SPLAY tree version as it consumes less
  memory and may even be faster in the average case. As pointers are
  managed, all additional information can go into the pointed-to
  structures.

  Copyright 1998, 1999 by the author.
  This code is released under the GNU General Public Licence.
  See the file COPYING in the main CLIB directory for details.
  Run "eprover -h" for contact information.

Changes

<1> Thu Sep 25 02:36:58 MET DST 1997
    New
<2> Wed Jan 27 17:05:13 MET 1999
    Moved over to D. Sleators splay trees. You can find the old code
    in clb_ptrees_avl.[ch] at the moment.

-----------------------------------------------------------------------*/

#ifndef CLB_PTREES

#define CLB_PTREES

#include <clb_pstacks.h>
#include <clb_avlgeneric.h>

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Data structure for indexing pointers (which need to be casted
   carefully by the wrapper functions). */ 

typedef struct ptreecell
{
   void*            key;
   struct ptreecell *lson;
   struct ptreecell *rson;
}PTreeCell, *PTree_p;



/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define PTreeCellAlloc()    (PTreeCell*)SizeMalloc(sizeof(PTreeCell))
#define PTreeCellFree(junk) SizeFree(junk, sizeof(PTreeCell))

#ifdef CONSTANT_MEM_ESTIMATE
#define PTREE_CELL_MEM 16
#else
#define PTREE_CELL_MEM MEMSIZE(PTreeCell)
#endif

/* As I found out now, _if_ int or long, respectively, is large enough
   for this conversion, this is guaranteed to work! */
#ifdef P_EQUAL_INT
#define PCmp(p1, p2) ((int)(p1)-(int)(p2))
#elif P_EQUAL_LONG
#define PCmp(p1, p2) ((long)(p1)-(long)(p2))
#elif P_EQUAL_LONG_LONG
#define PCmp(p1, p2) ((long long)(p1)-(long long)(p2))
#else 
#define PCmp(p1, p2) PCmpFun(p1, p2) 
#endif

int     PCmpFun(void* p1, void*p2);
PTree_p PTreeCellAllocEmpty(void);
void    PTreeFree(PTree_p junk);
PTree_p PTreeInsert(PTree_p *root, PTree_p new);
bool    PTreeStore(PTree_p *root, void* key);
PTree_p PTreeFind(PTree_p *root, void* key);
PTree_p PTreeFindBinary(PTree_p root, void* key);
PTree_p PTreeExtractEntry(PTree_p *root, void* key);
void*   PTreeExtractKey(PTree_p *root, void* key);
void*   PTreeExtractRootKey(PTree_p *root);
bool    PTreeDeleteEntry(PTree_p *root, void* key);
bool    PTreeMerge(PTree_p *root, PTree_p add);
void    PTreeInsertTree(PTree_p *root, PTree_p add);
long    PTreeNodes(PTree_p root);
long    PTreeDebugPrint(FILE* out, PTree_p root);
long    PStackToPTree(PTree_p *root, PStack_p stack);
long    PTreeToPStack(PStack_p target_stack, PTree_p root);
void*   PTreeSharedElement(PTree_p *tree1, PTree_p tree2);
PTree_p PTreeIntersection(PTree_p tree1, PTree_p tree2);
long    PTreeDestrIntersection(PTree_p *tree1, PTree_p tree2);
PTree_p PTreeCopy(PTree_p tree1);


AVL_TRAVERSE_DECLARATION(PTree, PTree_p)
#define PTreeTraverseExit(stack) PStackFree(stack)

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





