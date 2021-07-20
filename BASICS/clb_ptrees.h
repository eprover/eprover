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
  This code is released under the GNU General Public Licence and
  the GNU Lesser General Public License.
  See the file COPYING in the main E directory for details..
  Run "eprover -h" for contact information.

  Changes

  Created: Thu Sep 25 02:36:58 MET DST 1997

-----------------------------------------------------------------------*/

#ifndef CLB_PTREES

#define CLB_PTREES

#include <stdint.h>
#include <clb_pstacks.h>
#include <clb_avlgeneric.h>


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/


/* Data structure for indexing pointers (which need to be casted
   carefully by the wrapper functions). The key comes last in the
   struct to circumvent some bug in various gcc versions (apparently,
   gcc likes to safe a variable and will not always allocate a
   temporary variable when it thinks it can reuse the original
   position. In this case, it is wrong (exhibited in
   PTreeExtractKey()). Moving key to the back works around it (the
   memory management module will overwrite just the first word...) */

typedef struct ptreecell
{
   struct ptreecell *lson;
   struct ptreecell *rson;
   void*            key;
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

#define PCmp(p1, p2)    PCmpFun(p1, p2)
#define PEqual(p1,p2)   ((uintptr_t)(p1))==((uintptr_t)(p2))
#define PGreater(p1,p2) ((uintptr_t)(p1))> ((uintptr_t)(p2))
#define PLesser(p1,p2)  ((uintptr_t)(p1))< ((uintptr_t)(p2))

static  inline int PCmpFun(const void* p1, const void*p2);
PTree_p PTreeCellAllocEmpty(void);
void    PTreeFree(PTree_p junk);
PTree_p PTreeInsert(PTree_p *root, PTree_p newnode);
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
bool    PTreeEquiv(PTree_p t1, PTree_p t2);
bool    PTreeIsSubset(PTree_p sub, PTree_p *super);

void    PTreeVisitInOrder(PTree_p t, void (*visitor)(void*, void*), void* arg);

AVL_TRAVERSE_DECLARATION(PTree, PTree_p)
#define PTreeTraverseExit(stack) PStackFree(stack)


/*-----------------------------------------------------------------------
//
// Function: PCmpFun()
//
//   Compare two pointers, return 1 if the first one is bigger, 0 if
//   both are equal, and -1 if the second one is bigger.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static inline int PCmpFun(const void* p1, const void*p2)
{
   return ((uintptr_t)p1 > (uintptr_t)p2) - ((uintptr_t)p1 < (uintptr_t)p2);
}


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/
