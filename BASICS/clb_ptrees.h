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

/* As I found out now, _if_ ptr_int is large enough
   for this conversion, this is guaranteed to work! - actually it's
   does not if the pointers are great enough to overflow. "Slow"
   inlined is fast enough and seems to work fine. */

#define SLOW_PTR_CMP
#ifdef SLOW_PTR_CMP
#define PCmp(p1, p2) PCmpFun(p1, p2) 
#else
#define PCmp(p1, p2) ((ptr_int)(p1)-(ptr_int)(p2))
#endif

static  __inline__ int PCmpFun(void* p1, void*p2);
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


AVL_TRAVERSE_DECLARATION(PTree, PTree_p)
#define PTreeTraverseExit(stack) PStackFree(stack)


/*-----------------------------------------------------------------------
//
// Function: PCmpFun()
//
//   Compare two pointers, return 1 if the first one is bigger, 0 if
//   both are equal, and -1 if the second one is bigger. Might be
//   machine dependend and of limited portability (comparing two
//   arbitrary pointers is not ANSI kosher, but the compiler has no
//   way to detect this, as pointers to the same array can be compared
//   under ANSI), but should be easy to hack on any
//   machine. Subtracting pointers and casting the result to int is
//   probably more efficient, but even more dangerous.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

static int PCmpFun(void* p1, void*p2)
{
   if(p1 > p2)
   {
      return 1;
   }
   else if(p1 < p2)
   {
      return -1;
   }
   assert(p1 == p2);
   return 0;
}
      


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/





